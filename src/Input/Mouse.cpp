
#define _USE_MATH_DEFINES

#include "Input/Mouse.h"
#include "Config/Config.h"
#include "System/Helpers.h"
#include "cassert"
#include "imgui.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace Mouse {
std::array<MouseStyle, static_cast<std::size_t>(Type::maxMouseStyles)>
    mouseStyles{{{"Cat Board", Type::CatBoard},
                 {"Noh Board", Type::NohBoard},
                 {"Girl Glock", Type::GirlGlock}}};

std::array<TrailStyle, static_cast<std::size_t>(TrailType::maxTrailStyles)>
    trailStyles{
        {{"Circle", TrailType::Circle}, {"catmull", TrailType::Catmull}}};

MouseState g_mouseState{Config::activeConfig.theme.mouse.sensitivity,
                        Config::activeConfig.theme.mouse.decayRate,
                        Config::activeConfig.theme.mouse.fadeTime};
// helper
static float distance(const ImVec2 &v1, const ImVec2 &v2) {
  const float dx{v1.x - v2.x};
  const float dy{v1.y - v2.y};
  return std::sqrt(dx * dx + dy * dy);
}

static ImVec2 catmullRom(const ImVec2 &p0, const ImVec2 &p1, const ImVec2 &p2,
                         const ImVec2 &p3, float t) {
  const float t2 = t * t;
  const float t3 = t2 * t;
  return {0.5f * ((2.0f * p1.x) + (-p0.x + p2.x) * t +
                  (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
                  (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3),
          0.5f * ((2.0f * p1.y) + (-p0.y + p2.y) * t +
                  (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 +
                  (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3)};
}

// mousestate class
float MouseState::getDistance() const {
  return (std::sqrt(m_offsetX * m_offsetX + m_offsetY * m_offsetY));
}

void MouseState::addRawDelta(int dx, int dy) {
  m_pendingDeltaX += dx;
  m_pendingDeltaY += dy;
}

void MouseState::consumePendingDelta() {
  m_deltaX = m_pendingDeltaX;
  m_deltaY = m_pendingDeltaY;
  m_pendingDeltaX = 0;
  m_pendingDeltaY = 0;
}

void offsetRadiusClamp(float dist, float radius, float &outOffsetX,
                       float &outOffsetY) {
  if (dist > radius) {
    float scale{radius / dist};
    outOffsetX *= scale;
    outOffsetY *= scale;
  }
}

bool offsetRectClamp(float halfWidth, float halfHeight, float &outOffsetX,
                     float &outOffsetY) {
  bool wrapped{false};

  if (outOffsetX > halfWidth) {
    outOffsetX = -halfWidth;
    wrapped = true;
  } else if (outOffsetX < -halfWidth) {
    outOffsetX = halfWidth;
    wrapped = true;
  }

  if (outOffsetY > halfHeight) {
    outOffsetY = -halfHeight;
    wrapped = true;
  } else if (outOffsetY < -halfHeight) {
    outOffsetY = halfHeight;
    wrapped = true;
  }

  return wrapped;
}

// circle
void MouseState::updateMouseState(float deltaTime, float radius) {
  m_sensitivity = Config::activeConfig.theme.mouse.sensitivity;
  m_decayRate = Config::activeConfig.theme.mouse.decayRate;

  m_offsetX += m_deltaX * m_sensitivity;
  m_offsetY += m_deltaY * m_sensitivity;

  float decay{std::exp(-m_decayRate * deltaTime)};
  m_offsetX *= decay;
  m_offsetY *= decay;

  offsetRadiusClamp(getDistance(), radius, m_offsetX, m_offsetY);
}

// rect
void MouseState::updateMouseState(float deltaTime, float halfWidth,
                                  float halfHeight) {
  m_sensitivity = Config::activeConfig.theme.mouse.sensitivity;
  m_fadeTime = Config::activeConfig.theme.mouse.fadeTime;
  m_offsetX += m_deltaX * m_sensitivity;
  m_offsetY += m_deltaY * m_sensitivity;

  if (m_deltaX != 0 || m_deltaY != 0)
    m_idleTime = 0.0f;
  else
    m_idleTime += deltaTime;

  if (m_idleTime >= m_fadeTime) {
    m_offsetX = 0.0f;
    m_offsetY = 0.0f;
  }
  m_justWrapped = offsetRectClamp(halfWidth, halfHeight, m_offsetX, m_offsetY);
}

void MouseState::updateTrail(float deltaTime, TrailType type) {
  switch (type) {
  case TrailType::Circle:
    updateCircleTrail(deltaTime);
    break;
  case TrailType::Catmull:
    updateCatmullTrail(deltaTime);
    break;
  default:
    break;
  }
}

void MouseState::updateCatmullTrail(float deltaTime) {
  const float maxAge{Config::activeConfig.theme.mouse.trail.duration};
  const float maxLength{Config::activeConfig.theme.mouse.trail.maxLength};

  for (auto &pt : m_trail) {
    pt.age += deltaTime;
  }
  while (!m_trail.empty() && m_trail.front().age > maxAge) {
    m_trail.pop_front();
  }
  if (m_deltaX == 0 && m_deltaY == 0) {
    return;
  }

  const ImVec2 current{m_offsetX, m_offsetY};
  float segmentLength{0.0f};

  if (!m_trail.empty() && !m_justWrapped) {
    segmentLength = distance(m_trail.back().pos, current);
  }

  m_trailTotalDist += segmentLength;

  m_trail.emplace_back(
      TrailPoint{current, 0.0f, m_trailTotalDist, m_justWrapped});

  m_justWrapped = false;

  while (m_trail.size() > 1) {
    const float newestDist{m_trail.back().distance};
    const float oldestDist{m_trail.front().distance};
    if (newestDist - oldestDist > maxLength) {
      m_trail.pop_front();
    } else {
      break;
    }
  }

  if (m_trail.empty()) {
    m_trailTotalDist = 0.0f;
  }
}

void MouseState::updateCircleTrail(float deltaTime) {
  float maxAge{Config::activeConfig.theme.mouse.trail.duration};
  for (auto &pt : m_trail) {
    pt.age += deltaTime;
  }
  while (!m_trail.empty() && m_trail.front().age > maxAge) {
    m_trail.pop_front();
  }
  if (m_deltaX != 0 || m_deltaY != 0) {
    m_trail.push_back({{m_offsetX, m_offsetY}, 0.0f});
  }
}

void MouseState::registerScroll(bool up) {
  (up ? m_scrollUpTimer : m_scrollDownTimer) = 0.15f; // seconds
}

void MouseState::updateScrollTimers(float dt) {
  m_scrollUpTimer = (std::max)(0.0f, m_scrollUpTimer - dt);
  m_scrollDownTimer = (std::max)(0.0f, m_scrollDownTimer - dt);
}

static ImU32 lerpColor(ImU32 colorA, ImU32 colorB, float t) {
  t = std::clamp(t, 0.0f, 1.0f);

  ImVec4 a = ImGui::ColorConvertU32ToFloat4(colorA);
  ImVec4 b = ImGui::ColorConvertU32ToFloat4(colorB);

  ImVec4 result{a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t};

  return ImGui::ColorConvertFloat4ToU32(result);
}

float getFade() {
  return std::clamp(
      (1.0f - g_mouseState.getIdleTime() / g_mouseState.getFadeDur()), 0.0f,
      1.0f);
}

ImU32 applyFade(const Config::ColorRGBA &initColor, const float fade) {
  ImVec4 initColorImVec{Helper::fromConfigColor(initColor)};
  initColorImVec.w = fade;

  return Config::ToImU32(Helper::toConfigColor(initColorImVec));
}

void renderCatmullTrail(const ImVec2 &center,
                        const Config::ColorRGBA &initColor) {
  ImDrawList *draw = ImGui::GetBackgroundDrawList();

  const auto &trail{g_mouseState.getTrail()};
  if (trail.empty()) {
    return;
  }
  const float maxAge{Config::activeConfig.theme.mouse.trail.duration};
  constexpr float maxWidth = 3.0f;
  constexpr float minWidth = 1.0f;
  constexpr int subdivisions = 5;
  const float fade{getFade()};

  std::vector<std::vector<ImVec2>> runs;
  runs.emplace_back();

  for (const auto &pt : trail) {
    if (pt.breakLine)
      runs.emplace_back();

    runs.back().emplace_back(ImVec2{center.x + pt.pos.x, center.y + pt.pos.y});
  }

  std::vector<ImVec2> points;
  points.reserve(trail.size());

  for (const auto &points : runs) {
    if (points.size() < 2)
      continue;

    std::vector<ImVec2> smoothPoints;
    smoothPoints.reserve((points.size() - 1) * subdivisions + 1);

    for (std::size_t i{0}; i + 1 < points.size(); ++i) {
      const ImVec2 &p0 = points[i == 0 ? i : i - 1];
      const ImVec2 &p1 = points[i];
      const ImVec2 &p2 = points[i + 1];
      const ImVec2 &p3 = points[(i + 2 < points.size()) ? i + 2 : i + 1];

      for (std::size_t s{0}; s < subdivisions; ++s) {
        const float t =
            static_cast<float>(s) / static_cast<float>(subdivisions);

        smoothPoints.emplace_back(catmullRom(p0, p1, p2, p3, t));
      }
    }

    for (std::size_t i{1}; i < smoothPoints.size(); ++i) {
      const float t{static_cast<float>(i) /
                    static_cast<float>(smoothPoints.size() - 1)};

      const float width{minWidth + (maxWidth - minWidth) * t};

      draw->AddLine(smoothPoints[i - 1], smoothPoints[i],
                    applyFade(initColor, fade), width);
    }
  }
}

void renderCircleMouseTrail(const ImVec2 &center,
                            const Config::ColorRGBA &initColor) {
  ImDrawList *draw = ImGui::GetBackgroundDrawList();

  const auto &trail{g_mouseState.getTrail()};

  if (trail.empty())
    return;

  const float maxAge{Config::activeConfig.theme.mouse.trail.duration};
  // const float maxLength{Config::activeConfig.theme.mouse.trail.maxLength};
  // todo move it to cfg
  constexpr float maxWidth{3.0f};
  constexpr float minWidth{1.0f};
  constexpr int subdivisions{5};

  float fade{getFade()};
  if (fade <= 0.0f)
    return;

  const ImU32 baseColor = applyFade(initColor, fade);

  for (const auto &pt : trail) {
    float t = std::clamp(1.0f - (pt.age / maxAge), 0.0f, 1.0f);
    float alpha = fade * t;

    ImU32 color =
        (baseColor & 0x00FFFFFF) | (static_cast<ImU32>(alpha * 255) << 24);
    float radius = 1.5f + 2.5f * t;

    draw->AddCircleFilled({center.x + pt.pos.x, center.y + pt.pos.y}, radius,
                          color);
  }
}

void renderMouseTrail(const ImVec2 &center, const Config::ColorRGBA &initColor,
                      float maxAge, TrailType type) {
  switch (type) {
  case TrailType::Circle:
    renderCircleMouseTrail(center, initColor);
    break;
  case TrailType::Catmull:
    renderCatmullTrail(center, initColor);
    break;
  default:
    break;
  }
}

void renderNohBoardMouseIndicator(const ImVec2 &center) {
  ImDrawList *draw{ImGui::GetBackgroundDrawList()};

  // change it to custom colors
  const ImU32 innerColor{
      Config::ToImU32(Config::activeConfig.theme.mouse.inner)};

  const ImU32 outerColor{
      Config::ToImU32(Config::activeConfig.theme.mouse.outer)};

  float radius{Config::activeConfig.theme.mouse.radius};

  // outer circle
  draw->AddCircle(center, radius, outerColor, 64,
                  Config::activeConfig.theme.mouse.outerThickness);

  const float offsetX{g_mouseState.getOffsetX()};
  const float offsetY{g_mouseState.getOffsetY()};

  const float distance{g_mouseState.getDistance()};

  const float indicatorLength{(std::min)(distance, radius)};

  if (indicatorLength > 1.0f) {

    const float angle{std::atan2(offsetY, offsetX)};

    // 0 = center 1 = edge

    const float colorMultiplier{
        std::clamp(indicatorLength / radius, 0.0f, 1.0f)};

    const ImU32 indicatorColor{
        Config::activeConfig.theme.mouse.lerpColor
            ? lerpColor(innerColor, outerColor, colorMultiplier)
            : innerColor};

    constexpr float halfAngle{10.0f * static_cast<float>(M_PI) / 180.0f};
    constexpr int segments{16};

    const float startAngle{angle - halfAngle};
    const float endAngle{angle + halfAngle};

    ImVector<ImVec2> points;
    points.reserve(segments + 2);

    points.push_back(center);

    for (int i = 0; i <= segments; ++i) {
      const float t{static_cast<float>(i) / segments};

      // linear interpolation
      const float currentAngle{startAngle + (endAngle - startAngle) * t};

      points.push_back({center.x + std::cos(currentAngle) * indicatorLength,
                        center.y + std::sin(currentAngle) * indicatorLength});
    }

    draw->AddConvexPolyFilled(points.Data, points.Size, indicatorColor);

    const float smallRadius{Config::activeConfig.theme.mouse.smallRadius};

    ImVec2 directionEdge{center.x + std::cos(angle) * radius,
                         center.y + std::sin(angle) * radius};

    draw->AddCircleFilled(directionEdge, smallRadius, indicatorColor);
  }

  const float smallRadius{Config::activeConfig.theme.mouse.smallRadius};

  draw->AddCircleFilled(center, smallRadius, innerColor);
}

void renderDefaultMouseIndicator(const ImVec2 &center) {
  float radius{Config::activeConfig.theme.mouse.radius};
  ImDrawList *draw = ImGui::GetBackgroundDrawList();

  const ImU32 innerColor{
      Config::ToImU32(Config::activeConfig.theme.mouse.inner)};

  const ImU32 outerColor{
      Config::ToImU32(Config::activeConfig.theme.mouse.outer)};

  const float indicatorLength{(std::min)(g_mouseState.getDistance(), radius)};

  const float colorMultiplier{std::clamp(indicatorLength / radius, 0.0f, 1.0f)};

  const ImU32 indicatorColor{
      Config::activeConfig.theme.mouse.lerpColor
          ? lerpColor(innerColor, outerColor, colorMultiplier)
          : innerColor};

  draw->AddCircle(center, radius, outerColor, 64,
                  Config::activeConfig.theme.mouse.outerThickness);

  ImVec2 dotPos{center.x + g_mouseState.getOffsetX(),
                center.y + g_mouseState.getOffsetY()};

  draw->AddCircleFilled(dotPos, Config::activeConfig.theme.mouse.smallRadius,
                        indicatorColor);
}

void renderGirlGlockIndicator(const ImVec2 &center) {
  float width{Config::activeConfig.theme.mouse.width};
  float height{Config::activeConfig.theme.mouse.height};
  float halfWidth{width / 2};
  float halfHeight{height / 2};

  ImDrawList *draw = ImGui::GetBackgroundDrawList();

  float fade{getFade()};

  const ImU32 innerColor{
      applyFade(Config::activeConfig.theme.mouse.inner, fade)};

  const ImU32 outerColor{
      applyFade(Config::activeConfig.theme.mouse.outer, fade)};

  const ImU32 outlineColor{
      Config::ToImU32(Config::activeConfig.theme.mouse.outer)};

  const ImU32 fillColor{Config::ToImU32(Config::activeConfig.theme.mouse.fill)};

  const float colorMultiplier{
      std::clamp((std::max)(std::abs(g_mouseState.getOffsetX()) / halfWidth,
                            std::abs(g_mouseState.getOffsetY()) / halfHeight),
                 0.0f, 1.0f)};

  const ImU32 indicatorColor{
      Config::activeConfig.theme.mouse.lerpColor
          ? lerpColor(innerColor, outerColor, colorMultiplier)
          : innerColor};

  float rounding{Config::activeConfig.theme.mouse.rounding};
  float thickness{Config::activeConfig.theme.mouse.outerThickness};

  ImVec2 topLeft(center.x - halfWidth, center.y - halfHeight);
  ImVec2 bottomRight(center.x + halfWidth, center.y + halfHeight);

  // ImVec2 outlineTopLeft(center.x - halfWidth - thickness, center.y -
  // halfHeight - thickness); ImVec2 outlineBottomRight(center.x + halfWidth +
  // thickness, center.y + halfHeight + thickness);

  draw->AddRectFilled(topLeft, bottomRight, fillColor, rounding);

  ImVec2 dotPos{center.x + g_mouseState.getOffsetX(),
                center.y + g_mouseState.getOffsetY()};

  renderMouseTrail(center, Config::activeConfig.theme.mouse.trail.color,
                   Config::activeConfig.theme.mouse.trail.duration,
                   Config::activeConfig.theme.mouse.trail.style.type);

  draw->AddCircleFilled(dotPos, Config::activeConfig.theme.mouse.smallRadius,
                        indicatorColor);

  draw->AddRect(topLeft, bottomRight, outlineColor, rounding, 0, thickness);
}

void renderMouseIndicator(const ImVec2 &center,
                          const Mouse::Type mouseStyleType, const bool trail) {
  switch (mouseStyleType) {
  case Type::CatBoard:
    renderDefaultMouseIndicator(center);
    break;
  case Type::NohBoard:
    renderNohBoardMouseIndicator(center);
    break;
  case Type::GirlGlock:
    renderGirlGlockIndicator(center);
  default:
    break;
  }
}

} // namespace Mouse

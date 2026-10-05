
#define _USE_MATH_DEFINES

#include "Input/Mouse.h"
#include "UI/Layouts.h"
#include "Config/Config.h"
#include "System/Helpers.h"
#include "cassert"
#include "imgui.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace Mouse {
static bool s_m1Highlighted{false};

std::array<MouseStyle, static_cast<std::size_t>(Type::maxMouseStyles)>
    mouseStyles{{{"Cat Board", Type::CatBoard},
                 {"Noh Board", Type::NohBoard},
                 {"Wrap", Type::Wrap},
                 {"Pan", Type::Pan}}};

std::array<TrailStyle, static_cast<std::size_t>(TrailType::maxTrailStyles)>
    trailStyles{
        {{"Circle", TrailType::Circle}, {"Catmull", TrailType::Catmull}}};

MouseState g_mouseState{Config::activeConfig.theme.mouse.sensitivity,
                        Config::activeConfig.theme.mouse.decayRate,
                        Config::activeConfig.theme.mouse.fadeTime};
//extern helpers
bool isTypeCircle(Mouse::Type type) {
  return (type == Type::CatBoard || type == Type::NohBoard);
}
bool isTypeRect(Mouse::Type type) {
  return (type == Type::Wrap || type == Type::Pan);
}

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
  const auto &mouse{Config::activeConfig.theme.mouse};
  m_sensitivity = mouse.sensitivity;
  m_fadeTime = mouse.fadeTime;
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
  if (mouse.style.type != Type::Pan) {
    m_justWrapped =
        offsetRectClamp(halfWidth, halfHeight, m_offsetX, m_offsetY);
  }
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

  const ImVec2 current{trailPos()};
  float segmentLength{0.0f};

  if (!m_trail.empty() && !m_justWrapped) {
    segmentLength = distance(m_trail.back().pos, current);
  }

  m_trailTotalDist += segmentLength;

  m_trail.emplace_back(TrailPoint{current, 0.0f, m_trailTotalDist,
                                  m_justWrapped, s_m1Highlighted});

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

// todo update this func
void MouseState::updateCircleTrail(float deltaTime) {
  float maxAge{Config::activeConfig.theme.mouse.trail.duration};
  for (auto &pt : m_trail) {
    pt.age += deltaTime;
  }
  while (!m_trail.empty() && m_trail.front().age > maxAge) {
    m_trail.pop_front();
  }
  if (m_deltaX != 0 || m_deltaY != 0) {
    m_trail.emplace_back(
        TrailPoint{{m_offsetX, m_offsetY}, 0.0f, 0.0f, false, s_m1Highlighted});
  }
}

void MouseState::registerScroll(int wheelDelta) {
  m_pendingScroll += wheelDelta;
}

void MouseState::updateScroll(float dt) {
  constexpr float holdTime{0.15f};

  const int delta{m_pendingScroll};
  m_pendingScroll = 0;

  if (delta > 0) {
    m_scrollUpValue += delta;
    m_scrollUpTimer = holdTime;
  } else if (delta < 0) {
    m_scrollDownValue += delta;
    m_scrollDownTimer = holdTime;
  }

  m_scrollUpTimer = (std::max)(0.0f, m_scrollUpTimer - dt);
  m_scrollDownTimer = (std::max)(0.0f, m_scrollDownTimer - dt);

  if (m_scrollUpTimer <= 0.0f)
    m_scrollUpValue = 0;
  if (m_scrollDownTimer <= 0.0f)
    m_scrollDownValue = 0;
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

  return Helper::ToImU32(Helper::toConfigColor(initColorImVec));
}

void updateM1Highlight(const std::array<bool, 256> &currentFullKeysDown) {
  const auto &mouse{Config::activeConfig.theme.mouse};
  s_m1Highlighted = mouse.highlightM1 && currentFullKeysDown[VK_LBUTTON];
}

static float highlightScale() {
  return s_m1Highlighted ? Config::activeConfig.theme.mouse.higlightScale
                         : 1.0f;
}

static const Config::ColorRGBA &inner() {
  const auto &mouse{Config::activeConfig.theme.mouse};
  return s_m1Highlighted ? mouse.highlightColor : mouse.inner;
}

struct RunPoint {
  ImVec2 pos;
  bool hl;
};

void renderCatmullTrail(const ImVec2 &center) {
  ImDrawList *draw = ImGui::GetBackgroundDrawList();
  const auto &mouse{Config::activeConfig.theme.mouse};
  const auto &trail{Config::activeConfig.theme.mouse.trail};

  const auto &trailPts{g_mouseState.getTrailPoints()};
  if (trailPts.empty()) {
    return;
  }

  const float fade{getFade()};
  const ImU32 normalColor{applyFade(trail.color, fade)};
  const ImU32 hlColor{applyFade(mouse.highlightColor, fade)};

  if (fade <= 0.0f) {
    return;
  }

  if (trailPts.size() == 1) {
    const ImVec2 pos{center.x + trailPts.front().pos.x,
                     center.y + trailPts.front().pos.y};

    draw->AddCircleFilled(pos, trail.minWidth, normalColor);

    return;
  }

  std::vector<std::vector<RunPoint>> runs;
  runs.emplace_back();

  for (const auto &pt : trailPts) {
    if (pt.breakLine)
      runs.emplace_back();

    runs.back().emplace_back(ImVec2{center.x + pt.pos.x, center.y + pt.pos.y},
                             pt.highlighted);
  }

  for (const auto &points : runs) {
    if (points.size() < 2)
      continue;

    std::vector<RunPoint> smoothPoints;
    smoothPoints.reserve((points.size() - 1) * trail.subdivisions + 1);

    for (std::size_t i{0}; i + 1 < points.size(); ++i) {
      const ImVec2 &p0 = points[i == 0 ? i : i - 1].pos;
      const ImVec2 &p1 = points[i].pos;
      const ImVec2 &p2 = points[i + 1].pos;
      const ImVec2 &p3 = points[(i + 2 < points.size()) ? i + 2 : i + 1].pos;

      for (std::size_t s{0}; s < trail.subdivisions; ++s) {
        const float t =
            static_cast<float>(s) / static_cast<float>(trail.subdivisions);

        const bool hl{s == 0 ? points[i].hl : points[i + 1].hl};
        smoothPoints.emplace_back(RunPoint{catmullRom(p0, p1, p2, p3, t), hl});
      }
    }

    for (std::size_t i{1}; i < smoothPoints.size(); ++i) {
      const float t{static_cast<float>(i) /
                    static_cast<float>(smoothPoints.size() - 1)};

      float width{trail.minWidth + (trail.maxWidth - trail.minWidth) * t};
      const bool hl{smoothPoints[i].hl};
      if (hl)
        width *= mouse.higlightScale;

      draw->AddLine(smoothPoints[i - 1].pos, smoothPoints[i].pos,
                    hl ? hlColor : normalColor, width);
    }

    // const ImVec2 &tip{smoothPoints.back()};
    //
    // draw->AddCircleFilled(tip, maxWidth, applyFade(trail.color, fade));
  }
}

void renderCircleMouseTrail(const ImVec2 &center) {
  ImDrawList *draw = ImGui::GetBackgroundDrawList();
  const auto &mouse{Config::activeConfig.theme.mouse};
  const auto &trail{Config::activeConfig.theme.mouse.trail};

  const auto &trailPts{g_mouseState.getTrailPoints()};

  if (trailPts.empty())
    return;

  float fade{getFade()};
  if (fade <= 0.0f)
    return;

  const ImU32 normalColor{applyFade(trail.color, fade)};
  const ImU32 hlColor{applyFade(mouse.highlightColor, fade)};

  for (const auto &pt : trailPts) {
    float t = std::clamp(1.0f - (pt.age / trail.duration), 0.0f, 1.0f);
    float alpha = fade * t;

    const ImU32 baseColor{pt.highlighted ? hlColor : normalColor};
    ImU32 color =
        (baseColor & 0x00FFFFFF) | (static_cast<ImU32>(alpha * 255) << 24);

    float radius{trail.minWidth + trail.maxWidth * t};
    if (pt.highlighted)
      radius *= mouse.higlightScale;

    draw->AddCircleFilled({center.x + pt.pos.x, center.y + pt.pos.y}, radius,
                          color);
  }
}

void renderMouseTrail(const ImVec2 &center, TrailType type) {
  if (!Config::activeConfig.theme.mouse.trail.enable) {
    return;
  }
  switch (type) {
  case TrailType::Circle:
    renderCircleMouseTrail(center);
    break;
  case TrailType::Catmull:
    renderCatmullTrail(center);
    break;
  default:
    break;
  }
}

void renderNohBoardMouseIndicator(const ImVec2 &center) {
  ImDrawList *draw{ImGui::GetBackgroundDrawList()};
  const auto &mouse{Config::activeConfig.theme.mouse};

  const ImU32 innerColor{Helper::ToImU32(inner())};
  const ImU32 outerColor{Helper::ToImU32(mouse.outer)};
  float radius{Layouts::activeLayout.mouse.radius};

  // outer circle
  draw->AddCircleFilled(center, radius, Helper::ToImU32(mouse.fill));
  draw->AddCircle(center, radius, outerColor, 64, mouse.outerThickness);

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
        mouse.lerpColor ? lerpColor(innerColor, outerColor, colorMultiplier)
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
      // lerp
      const float currentAngle{startAngle + (endAngle - startAngle) * t};

      points.push_back({center.x + std::cos(currentAngle) * indicatorLength,
                        center.y + std::sin(currentAngle) * indicatorLength});
    }

    draw->AddConvexPolyFilled(points.Data, points.Size, indicatorColor);

    const float smallRadius{mouse.smallRadius * highlightScale()};

    ImVec2 directionEdge{center.x + std::cos(angle) * radius,
                         center.y + std::sin(angle) * radius};

    draw->AddCircleFilled(directionEdge, smallRadius, indicatorColor);
  }

  // not applying highlight scale here, u can if u want
  draw->AddCircleFilled(center, mouse.smallRadius, innerColor);
}

void renderDefaultMouseIndicator(const ImVec2 &center) {
  const auto &mouse{Config::activeConfig.theme.mouse};
  float radius{Layouts::activeLayout.mouse.radius};
  ImDrawList *draw = ImGui::GetBackgroundDrawList();

  const ImU32 innerColor{Helper::ToImU32(inner())};
  const ImU32 outerColor{Helper::ToImU32(mouse.outer)};
  const float indicatorLength{(std::min)(g_mouseState.getDistance(), radius)};
  const float colorMultiplier{std::clamp(indicatorLength / radius, 0.0f, 1.0f)};

  const ImU32 indicatorColor{
      mouse.lerpColor ? lerpColor(innerColor, outerColor, colorMultiplier)
                      : innerColor};

  draw->AddCircleFilled(center, radius, Helper::ToImU32(mouse.fill));
  draw->AddCircle(center, radius, outerColor, 64, mouse.outerThickness);

  renderMouseTrail(center, mouse.trail.style.type);
  ImVec2 dotPos{center.x + g_mouseState.getOffsetX(),
                center.y + g_mouseState.getOffsetY()};

  draw->AddCircleFilled(dotPos, mouse.smallRadius * highlightScale(),
                        indicatorColor);
}

void renderWrapIndicator(const ImVec2 &center) {
  const auto &mouse{Config::activeConfig.theme.mouse};
  const auto &layout{Layouts::activeLayout};

  const float halfWidth{layout.mouse.width / 2};
  const float halfHeight{layout.mouse.height / 2};

  ImDrawList *draw = ImGui::GetBackgroundDrawList();

  float fade{getFade()};

  const ImU32 innerColor{applyFade(inner(), fade)};
  const ImU32 outerColor{applyFade(mouse.outer, fade)};
  const ImU32 outlineColor{Helper::ToImU32(mouse.outer)};
  const ImU32 fillColor{Helper::ToImU32(mouse.fill)};

  const float colorMultiplier{
      std::clamp((std::max)(std::abs(g_mouseState.getOffsetX()) / halfWidth,
                            std::abs(g_mouseState.getOffsetY()) / halfHeight),
                 0.0f, 1.0f)};

  const ImU32 indicatorColor{
      mouse.lerpColor ? lerpColor(innerColor, outerColor, colorMultiplier)
                      : innerColor};

  ImVec2 topLeft(center.x - halfWidth, center.y - halfHeight);
  ImVec2 bottomRight(center.x + halfWidth, center.y + halfHeight);

  draw->AddRectFilled(topLeft, bottomRight, fillColor, mouse.rounding);

  ImVec2 dotPos{center.x + g_mouseState.getOffsetX(),
                center.y + g_mouseState.getOffsetY()};

  draw->PushClipRect(topLeft, bottomRight, true);

  renderMouseTrail(center, mouse.trail.style.type);
  draw->AddCircleFilled(dotPos, mouse.smallRadius * highlightScale(),
                        indicatorColor);

  draw->PopClipRect();

  draw->AddRect(topLeft, bottomRight, outlineColor, mouse.rounding, 0,
                mouse.outerThickness);
}

void renderPanIndicator(const ImVec2 &center) {
  const auto &mouse{Config::activeConfig.theme.mouse};
  const auto &layout{Layouts::activeLayout};

  const float halfWidth{layout.mouse.width / 2};
  const float halfHeight{layout.mouse.height / 2};

  ImDrawList *draw = ImGui::GetBackgroundDrawList();

  const float fade{getFade()};

  const ImU32 innerColor{applyFade(inner(), fade)};
  const ImU32 outlineColor{Helper::ToImU32(mouse.outer)};
  const ImU32 fillColor{Helper::ToImU32(mouse.fill)};

  ImVec2 topLeft(center.x - halfWidth, center.y - halfHeight);
  ImVec2 bottomRight(center.x + halfWidth, center.y + halfHeight);

  draw->AddRectFilled(topLeft, bottomRight, fillColor, mouse.rounding);

  const ImVec2 origin{center.x - g_mouseState.getOffsetX(),
                      center.y - g_mouseState.getOffsetY()};

  draw->PushClipRect(topLeft, bottomRight, true);
  renderMouseTrail(origin, mouse.trail.style.type);
  draw->PopClipRect();

  draw->AddCircleFilled(center, mouse.smallRadius * highlightScale(),
                        innerColor);

  draw->AddRect(topLeft, bottomRight, outlineColor, mouse.rounding, 0,
                mouse.outerThickness);
}

void renderMouseIndicator(const ImVec2 &center,
                          const Mouse::Type mouseStyleType) {
  switch (mouseStyleType) {
  case Type::CatBoard:
    renderDefaultMouseIndicator(center);
    break;
  case Type::NohBoard:
    renderNohBoardMouseIndicator(center);
    break;
  case Type::Wrap:
    renderWrapIndicator(center);
    break;
  case Type::Pan:
    renderPanIndicator(center);
    break;
  default:
    break;
  }

  // const auto &trail{Config::activeConfig.theme.mouse.trail};
  // if (trail.enable) {
  //   renderMouseTrail(center, trail.style.type);
  // }
}

} // namespace Mouse


#include <cassert>
#define _USE_MATH_DEFINES
#include "Config/Config.h"
#include "Input/Mouse.h"
#include "cassert"
#include "imgui.h"
#include <algorithm>
#include <cmath>

namespace Mouse {
std::array<MouseStyle, static_cast<std::size_t>(Type::maxMouseStyles)>
    mouseStyles{{{"Cat Board", Type::CatBoard}, {"Noh Board", Type::NohBoard}}};

MouseState g_mouseState{Config::activeConfig.theme.mouse.sensitivity,
                        Config::activeConfig.theme.mouse.decayRate};

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

void MouseState::updateMouseState(float deltaTime, float radius) {
  m_sensitivity = Config::activeConfig.theme.mouse.sensitivity;
  m_decayRate = Config::activeConfig.theme.mouse.decayRate;

  m_offsetX += m_deltaX * m_sensitivity;
  m_offsetY += m_deltaY * m_sensitivity;

  float decay{std::exp(-m_decayRate * deltaTime)};
  m_offsetX *= decay;
  m_offsetY *= decay;

  float dist{getDistance()};

  if (dist > radius) {
    float scale{radius / dist};
    m_offsetX *= scale;
    m_offsetY *= scale;
  }
}

void MouseState::updateTrail(float deltaTime, float maxAge) {
  for (auto &pt : m_trail) {
    pt.age += deltaTime;
  }
  while (!m_trail.empty() && m_trail.front().age > maxAge) {
    m_trail.pop_front();
  }
  m_trail.push_back({m_offsetX, m_offsetY, 0.0f});
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

void renderNohBoardMouseIndicator(const ImVec2 &center) {
  ImDrawList *draw{ImGui::GetBackgroundDrawList()};

  // change it to custom colors
  const ImU32 innerColor{
      Config::ToImU32(Config::activeConfig.theme.mouse.innerCircle)};

  const ImU32 outerColor{
      Config::ToImU32(Config::activeConfig.theme.mouse.outerCircle)};

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
      Config::ToImU32(Config::activeConfig.theme.mouse.innerCircle)};

  const ImU32 outerColor{
      Config::ToImU32(Config::activeConfig.theme.mouse.outerCircle)};

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

  draw->AddCircleFilled(dotPos,
                        Config::activeConfig.theme.mouse.smallRadius,
                        indicatorColor);
}

void renderMouseTrail(const ImVec2 &center, ImU32 baseColor, float maxAge) {
  ImDrawList *draw = ImGui::GetBackgroundDrawList();

  for (const auto &pt : Mouse::g_mouseState.getTrail()) {
    float t = std::clamp(1.0f - (pt.age / maxAge), 0.0f, 1.0f);

    ImU32 color =
        (baseColor & 0x00FFFFFF) | (static_cast<ImU32>(t * 255) << 24);
    float radius = 1.5f + 2.5f * t;

    draw->AddCircleFilled({center.x + pt.x, center.y + pt.y}, radius, color);
  }
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
  default:
    break;
  }
}

} // namespace Mouse

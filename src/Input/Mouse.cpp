#define _USE_MATH_DEFINES

#include "Input/Mouse.h"
#include "Config/Config.h"
#include "System/Helpers.h"
#include "UI/Layouts.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <vector>

#include <cstdio>

namespace Mouse {

namespace {

constexpr float cornerEpsilon{0.01f};
constexpr float minStepLength{0.0001f};
constexpr int boundarySearchIterations{12};

struct RunPoint {
  ImVec2 pos;
  bool highlighted;
};

using Run = std::vector<RunPoint>;

struct RectBounds {
  float halfWidth;
  float halfHeight;
  ImVec2 topLeft;
  ImVec2 bottomRight;
};

} // namespace

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

bool isTypeCircle(Mouse::Type type) {
  return (type == Type::CatBoard || type == Type::NohBoard);
}
bool isTypeRect(Mouse::Type type) {
  return (type == Type::Wrap || type == Type::Pan);
}

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

RoundedRect RoundedRect::make(float halfWidth, float halfHeight,
                              float rounding) {
  const float radius{
      (std::max)(0.0f, (std::min)({rounding, halfWidth, halfHeight}))};
  return {halfWidth, halfHeight, radius};
}

RoundedRect RoundedRect::shrunk(float amount) const {
  return make((std::max)(0.0f, halfWidth - amount),
              (std::max)(0.0f, halfHeight - amount), radius - amount);
}

bool RoundedRect::contains(const ImVec2 &point) const {
  const float overshootX{
      (std::max)(std::abs(point.x) - (halfWidth - radius), 0.0f)};
  const float overshootY{
      (std::max)(std::abs(point.y) - (halfHeight - radius), 0.0f)};
  return overshootX * overshootX + overshootY * overshootY <= radius * radius;
}

ImVec2 RoundedRect::boundaryAlong(const ImVec2 &inside,
                                  const ImVec2 &direction) const {
  const float reach{2.0f * (halfWidth + halfHeight)};
  ImVec2 lastInside{inside};
  ImVec2 firstOutside{inside.x + direction.x * reach,
                      inside.y + direction.y * reach};

  for (int i{0}; i < boundarySearchIterations; ++i) {
    const ImVec2 middle{(lastInside.x + firstOutside.x) * 0.5f,
                        (lastInside.y + firstOutside.y) * 0.5f};
    if (contains(middle))
      lastInside = middle;
    else
      firstOutside = middle;
  }
  return lastInside;
}

static bool wrapAcrossCorner(const RoundedRect &shape, ImVec2 &point) {
  if (shape.radius <= 0.0f)
    return false;

  // corner = center of rounding
  const float cornerX{shape.halfWidth - shape.radius};
  const float cornerY{shape.halfHeight - shape.radius};
  const float dx{std::abs(point.x) - cornerX};
  const float dy{std::abs(point.y) - cornerY};

  if (dx <= 0.0f || dy <= 0.0f)
    return false;

  const float distanceFromCorner{std::sqrt(dx * dx + dy * dy)};
  if (distanceFromCorner <= shape.radius)
    return false;

  const float toArc{(shape.radius - cornerEpsilon) / distanceFromCorner};
  const ImVec2 arcPoint{(cornerX + dx * toArc), (cornerY + dy * toArc)};
  const float signX{std::copysign(1.0f, point.x)};
  const float signY{std::copysign(1.0f, point.y)};

  const bool exitedThroughSide{dx > dy};
  point = exitedThroughSide ? ImVec2{-signX * arcPoint.x, signY * arcPoint.y}
                            : ImVec2{signX * arcPoint.x, -signY * arcPoint.y};
  return true;
}

static bool wrapAcrossEdges(const RoundedRect &shape, ImVec2 &point) {
  bool wrapped{false};

  if (std::abs(point.x) > shape.halfWidth) {
    point.x = -std::copysign(shape.halfWidth, point.x);
    wrapped = true;
  }
  if (std::abs(point.y) > shape.halfHeight) {
    point.y = -std::copysign(shape.halfHeight, point.y);
    wrapped = true;
  }
  return wrapped;
}

static bool wrapInsideRoundedRect(const RoundedRect &shape, ImVec2 &point) {
  return wrapAcrossCorner(shape, point) || wrapAcrossEdges(shape, point);
}

static void clampToRadius(float radius, float &offsetX, float &offsetY) {
  const float length{std::sqrt(offsetX * offsetX + offsetY * offsetY)};
  if (length > radius) {
    const float scale{radius / length};
    offsetX *= scale;
    offsetY *= scale;
  }
}

float MouseState::getDistance() const {
  return std::sqrt(m_offsetX * m_offsetX + m_offsetY * m_offsetY);
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

// round
void MouseState::updateMouseState(float deltaTime, float radius) {
  m_sensitivity = Config::activeConfig.theme.mouse.sensitivity;
  m_decayRate = Config::activeConfig.theme.mouse.decayRate;

  m_offsetX += m_deltaX * m_sensitivity;
  m_offsetY += m_deltaY * m_sensitivity;

  const float decay{std::exp(-m_decayRate * deltaTime)};
  m_offsetX *= decay;
  m_offsetY *= decay;

  clampToRadius(radius, m_offsetX, m_offsetY);
}

// rect
void MouseState::updateMouseState(float deltaTime, float halfWidth,
                                  float halfHeight) {
  const auto &mouse{Config::activeConfig.theme.mouse};
  m_sensitivity = mouse.sensitivity;
  m_fadeTime = mouse.fadeTime;
  m_shape = RoundedRect::make(halfWidth, halfHeight, mouse.rounding);

  m_offsetX += m_deltaX * m_sensitivity;
  m_offsetY += m_deltaY * m_sensitivity;

  if (m_deltaX != 0 || m_deltaY != 0)
    m_idleTime = 0.0f;
  else
    m_idleTime += deltaTime;

  if (m_idleTime >= m_fadeTime) {
    m_offsetX = 0.0f;
    m_offsetY = 0.0f;
    m_trail.clear();
  }

  if (mouse.style.type == Type::Pan)
    return;

  ImVec2 offset{m_offsetX, m_offsetY};
  const ImVec2 offsetBeforeWrap{offset};

  if (!wrapInsideRoundedRect(m_shape.shrunk(mouse.smallRadius), offset))
    return;

  m_offsetX = offset.x;
  m_offsetY = offset.y;
  m_justWrapped = true;
  m_wrapExit = offsetBeforeWrap;
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

void MouseState::ageTrail(float deltaTime) {
  const auto &mouse{Config::activeConfig.theme.mouse};

  // The Catmull trail uses fadeTime as maxAge to avoid an unnecessary
  // redundacy.(cuz lifetime already controlled by maxLength)
  // If you want to control its lifetime independently of fadeTime,
  // use trail.duration instead and add a corresponding slider for catmull to
  // SettingsWindow.
  const float maxAge{mouse.trail.style.type == TrailType::Circle
                         ? mouse.trail.duration
                         : mouse.fadeTime};

  for (auto &point : m_trail)
    point.age += deltaTime;

  while (!m_trail.empty() && m_trail.front().age > maxAge)
    m_trail.pop_front();

  if (m_trail.empty())
    m_trailTotalDist = 0.0f;
}

void MouseState::appendTrailPoint(const ImVec2 &pos, bool breakLine) {
  if (!m_trail.empty() && !breakLine)
    m_trailTotalDist += distance(m_trail.back().pos, pos);

  m_trail.emplace_back(
      TrailPoint{pos, 0.0f, m_trailTotalDist, breakLine, s_m1Highlighted});
}

void MouseState::appendWrapSegment(const ImVec2 &current) {
  const ImVec2 previous{m_trail.back().pos};
  const float stepLength{distance(m_wrapExit, previous)};

  if (stepLength < minStepLength) {
    appendTrailPoint(current, true);
    return;
  }

  const ImVec2 direction{(m_wrapExit.x - previous.x) / stepLength,
                         (m_wrapExit.y - previous.y) / stepLength};
  const ImVec2 exitPoint{m_shape.boundaryAlong(previous, direction)};

  const bool flippedX{(m_wrapExit.x < 0.0f) != (current.x < 0.0f)};
  const bool flippedY{(m_wrapExit.y < 0.0f) != (current.y < 0.0f)};

  const ImVec2 entryPoint{flippedX ? -exitPoint.x : exitPoint.x,
                          flippedY ? -exitPoint.y : exitPoint.y};

  appendTrailPoint(exitPoint, false);
  appendTrailPoint(entryPoint, true);
}

void MouseState::trimTrailToMaxLength(float maxLength) {
  while (m_trail.size() > 1 &&
         m_trail.back().distance - m_trail.front().distance > maxLength)
    m_trail.pop_front();
}

void MouseState::updateCatmullTrail(float deltaTime) {
  ageTrail(deltaTime);

  if (m_deltaX == 0 && m_deltaY == 0)
    return;

  const ImVec2 current{trailPos()};

  if (m_justWrapped && !m_trail.empty())
    appendWrapSegment(current);

  appendTrailPoint(current, false);
  m_justWrapped = false;

  trimTrailToMaxLength(Config::activeConfig.theme.mouse.trail.maxLength);
}

// todo update this func
void MouseState::updateCircleTrail(float deltaTime) {
  ageTrail(deltaTime);
  m_justWrapped = false;

  if (m_deltaX != 0 || m_deltaY != 0)
    m_trail.emplace_back(
        TrailPoint{trailPos(), 0.0f, 0.0f, false, s_m1Highlighted});
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

void updateM1Highlight(bool m1Pressed) {
  const auto &mouse{Config::activeConfig.theme.mouse};
  s_m1Highlighted = mouse.highlightM1 && m1Pressed;
}

static float highlightScale() {
  return s_m1Highlighted ? Config::activeConfig.theme.mouse.higlightScale
                         : 1.0f;
}

static const Config::ColorRGBA &inner() {
  const auto &mouse{Config::activeConfig.theme.mouse};
  return s_m1Highlighted ? mouse.highlightColor : mouse.inner;
}

static bool isVisible(const TrailClip *clip, const ImVec2 &trailPoint) {
  return !clip || clip->contains(trailPoint);
}

static std::vector<Run> splitIntoRuns(const std::deque<TrailPoint> &points,
                                      const ImVec2 &center,
                                      const TrailClip *clip) {
  std::vector<Run> runs(1);

  for (const auto &point : points) {
    const bool visible{isVisible(clip, point.pos)};
    const bool startsNewRun{point.breakLine || !visible};

    if (startsNewRun && !runs.back().empty())
      runs.emplace_back();

    if (visible)
      runs.back().push_back({{center.x + point.pos.x, center.y + point.pos.y},
                             point.highlighted});
  }
  return runs;
}

static Run smoothRun(const Run &run, std::size_t subdivisions) {
  Run smooth;
  smooth.reserve((run.size() - 1) * subdivisions + 1);

  for (std::size_t i{0}; i + 1 < run.size(); ++i) {
    const ImVec2 &before{run[i == 0 ? i : i - 1].pos};
    const ImVec2 &from{run[i].pos};
    const ImVec2 &to{run[i + 1].pos};
    const ImVec2 &after{run[i + 2 < run.size() ? i + 2 : i + 1].pos};

    for (std::size_t s{0}; s < subdivisions; ++s) {
      const float t{static_cast<float>(s) / static_cast<float>(subdivisions)};
      const bool highlighted{s == 0 ? run[i].highlighted
                                    : run[i + 1].highlighted};
      smooth.push_back({catmullRom(before, from, to, after, t), highlighted});
    }
  }

  smooth.push_back(run.back());
  return smooth;
}

static void renderCatmullTrail(const ImVec2 &center,
                               const TrailClip *clip = nullptr) {
  ImDrawList *draw{ImGui::GetBackgroundDrawList()};
  const auto &mouse{Config::activeConfig.theme.mouse};
  const auto &trail{mouse.trail};
  const auto &points{g_mouseState.getTrailPoints()};

  const float fade{getFade()};
  if (points.empty() || 0.0f >= fade)
    return;

  const ImU32 normalColor{applyFade(trail.color, fade)};
  const ImU32 highlightColor{applyFade(mouse.highlightColor, fade)};

  if (points.size() == 1) {
    const ImVec2 &only{points.front().pos};
    if (isVisible(clip, only))
      draw->AddCircleFilled({center.x + only.x, center.y + only.y},
                            trail.minWidth, normalColor);
    return;
  }

  for (const auto &run : splitIntoRuns(points, center, clip)) {
    if (run.size() < 2)
      continue;

    const Run smooth{
        smoothRun(run, static_cast<std::size_t>(trail.subdivisions))};

    for (std::size_t i{1}; i < smooth.size(); ++i) {
      const float progress{static_cast<float>(i) /
                           static_cast<float>(smooth.size() - 1)};
      float width{std::lerp(trail.minWidth, trail.maxWidth, progress)};
      if (smooth[i].highlighted)
        width *= mouse.higlightScale;

      draw->AddLine(smooth[i - 1].pos, smooth[i].pos,
                    smooth[i].highlighted ? highlightColor : normalColor,
                    width);
    }
  }
}

static void renderCircleTrail(const ImVec2 &center,
                              const TrailClip *clip = nullptr) {
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
    if (!isVisible(clip, pt.pos))
      continue;

    float t = std::clamp(1.0f - (pt.age / trail.duration), 0.0f, 1.0f);
    float alpha = fade * t;

    const ImU32 baseColor{pt.highlighted ? hlColor : normalColor};
    ImU32 color =
        (baseColor & 0x00FFFFFF) | (static_cast<ImU32>(alpha * 255) << 24);

    float radius{std::lerp(trail.minWidth, trail.maxWidth, t)};
    if (pt.highlighted)
      radius *= mouse.higlightScale;

    draw->AddCircleFilled({center.x + pt.pos.x, center.y + pt.pos.y}, radius,
                          color);
  }
}

static void renderTrail(const ImVec2 &center, TrailType type,
                        const TrailClip *clip = nullptr) {
  if (!Config::activeConfig.theme.mouse.trail.enable) {
    return;
  }
  switch (type) {
  case TrailType::Circle:
    renderCircleTrail(center, clip);
    break;
  case TrailType::Catmull:
    renderCatmullTrail(center, clip);
    break;
  default:
    break;
  }
}

static RectBounds rectBoundsAround(const ImVec2 &center) {
  const auto &size{Layouts::activeLayout.mouse};
  const float halfWidth{size.width / 2.0f};
  const float halfHeight{size.height / 2.0f};

  const auto &ovScale{Config::activeConfig.overlay.scale};
  return {
      halfWidth,
      halfHeight,
      {center.x - (halfWidth * ovScale), center.y - (halfHeight * ovScale)},
      {center.x + (halfWidth * ovScale), center.y + (halfHeight * ovScale)}};
}

static void drawRectFill(ImDrawList *draw, const RectBounds &bounds) {
  const auto &mouse{Config::activeConfig.theme.mouse};
  draw->AddRectFilled(bounds.topLeft, bounds.bottomRight,
                      Helper::ToImU32(mouse.fill), mouse.rounding);
}

static void drawRectOutline(ImDrawList *draw, const RectBounds &bounds) {
  const auto &mouse{Config::activeConfig.theme.mouse};
  draw->AddRect(bounds.topLeft, bounds.bottomRight,
                Helper::ToImU32(mouse.outer), mouse.rounding, 0,
                mouse.outerThickness);
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

  const float indicatorLength{(std::min)(g_mouseState.getDistance(), radius)};

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

  renderTrail(center, mouse.trail.style.type);
  ImVec2 dotPos{center.x + g_mouseState.getOffsetX(),
                center.y + g_mouseState.getOffsetY()};

  draw->AddCircleFilled(dotPos, mouse.smallRadius * highlightScale(),
                        indicatorColor);
}

void renderWrapIndicator(const ImVec2 &center) {
  const auto &mouse{Config::activeConfig.theme.mouse};
  const RectBounds bounds{rectBoundsAround(center)};
  ImDrawList *draw{ImGui::GetBackgroundDrawList()};

  const float fade{getFade()};
  const ImU32 innerColor{applyFade(inner(), fade)};
  const ImU32 outerColor{applyFade(mouse.outer, fade)};

  const float colorMultiplier{std::clamp(
      (std::max)(std::abs(g_mouseState.getOffsetX()) / bounds.halfWidth,
                 std::abs(g_mouseState.getOffsetY()) / bounds.halfHeight),
      0.0f, 1.0f)};

  const ImU32 indicatorColor{
      mouse.lerpColor ? lerpColor(innerColor, outerColor, colorMultiplier)
                      : innerColor};

  const ImVec2 dotPos{center.x + g_mouseState.getOffsetX(),
                      center.y + g_mouseState.getOffsetY()};
  const TrailClip clip{
      RoundedRect::make(bounds.halfWidth, bounds.halfHeight, mouse.rounding)};

  drawRectFill(draw, bounds);

  draw->PushClipRect(bounds.topLeft, bounds.bottomRight, true);
  renderTrail(center, mouse.trail.style.type, &clip);
  draw->AddCircleFilled(dotPos, mouse.smallRadius * highlightScale(),
                        indicatorColor);
  draw->PopClipRect();

  drawRectOutline(draw, bounds);
}

void renderPanIndicator(const ImVec2 &center) {
  const auto &mouse{Config::activeConfig.theme.mouse};
  const RectBounds bounds{rectBoundsAround(center)};
  ImDrawList *draw{ImGui::GetBackgroundDrawList()};

  const ImU32 innerColor{applyFade(inner(), getFade())};

  const ImVec2 trailOrigin{center.x - g_mouseState.getOffsetX(),
                           center.y - g_mouseState.getOffsetY()};
  const TrailClip clip{
      RoundedRect::make(bounds.halfWidth, bounds.halfHeight, mouse.rounding),
      {-g_mouseState.getOffsetX(), -g_mouseState.getOffsetY()}};

  drawRectFill(draw, bounds);

  draw->PushClipRect(bounds.topLeft, bounds.bottomRight, true);
  renderTrail(trailOrigin, mouse.trail.style.type, &clip);
  draw->PopClipRect();

  draw->AddCircleFilled(center, mouse.smallRadius * highlightScale(),
                        innerColor);

  drawRectOutline(draw, bounds);
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
}

} // namespace Mouse

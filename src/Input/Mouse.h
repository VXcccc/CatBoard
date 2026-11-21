#pragma once

#include "Config.h"
#include "imgui.h"
#include <array>
#include <deque>
#include <string>
#include <windows.h>

struct AppState;
namespace Config {
struct MouseTheme;
}

namespace Config {
using ColorRGBA = std::array<int, 4>;
struct Overlay;
} // namespace Config

namespace Mouse {

enum class Type { CatBoard, NohBoard, Wrap, Pan, maxMouseStyles };

struct MouseStyle {
  std::string name{"CatBoard"};
  Type type{Type::CatBoard};
};

extern const std::array<MouseStyle,
                        static_cast<std::size_t>(Type::maxMouseStyles)>
    mouseStyles;

enum class TrailType { Circle, Catmull, maxTrailStyles };

struct TrailStyle {
  std::string name{"Catmull"};
  TrailType type{TrailType::Catmull};
};

extern const std::array<TrailStyle,
                        static_cast<std::size_t>(TrailType::maxTrailStyles)>
    trailStyles;

struct TrailPoint {
  ImVec2 pos{};
  float age{0.0f};
  float distance{0.0f};
  bool breakLine{false};
  bool highlighted{false};
};

struct RoundedRect {
  float halfWidth{0.0f};
  float halfHeight{0.0f};
  float radius{0.0f};

  static RoundedRect make(float halfWidth, float halfHeight, float rounding);

  RoundedRect shrunk(float amount) const;
  bool contains(const ImVec2 &point) const;
  ImVec2 boundaryAlong(const ImVec2 &inside, const ImVec2 &direction) const;
};

struct TrailClip {
  RoundedRect shape;
  ImVec2 offset{0.0f, 0.0f};

  bool contains(const ImVec2 &trailPoint) const {
    return shape.contains({trailPoint.x + offset.x, trailPoint.y + offset.y});
  }
};

class State {
public:
  State() = default;

  State(float sensitivity, float decayRate, float fadeTime)
      : m_sensitivity(sensitivity), m_decayRate(decayRate),
        m_fadeTime(fadeTime) {}

  float getOffsetX() const { return m_offsetX; }
  float getOffsetY() const { return m_offsetY; }

  void setSensivity(float sensitivity) { m_sensitivity = sensitivity; }
  float getSensivity() const { return m_sensitivity; }

  void setDecayRate(float decayRate) { m_decayRate = decayRate; }
  float getDecayRate() const { return m_decayRate; }
  void setFadeTime(float fadeTime) { m_fadeTime = fadeTime; }
  void setM1Highlighted(bool highlighted) { m_m1Highlighted = highlighted; }
  bool isM1Highlighted() const { return m_m1Highlighted; }

  void updateMouseState(float deltaTime, float radius,
                        const Config::MouseTheme &mouse, float ovScale);
  void updateMouseState(float deltaTime, float halfWidth, float halfHeight,
                        const Config::MouseTheme &mouse, float ovScale);

  bool updateColors(float deltaTime, const Config::MouseTheme &theme,
                    bool hideOverlay, int hideAnimSpeed);

  float getDistance() const;

  float getIdleTime() const { return m_idleTime; }
  float getFadeDur() const { return m_fadeTime; }

  void addRawDelta(int dx, int dy);
  void consumePendingDelta();

  void updateTrail(float deltaTime, const Config::MouseTheme &mouse,
                   float ovScale);
  const std::deque<TrailPoint> &getTrailPoints() const { return m_trail; }

  void registerScroll(int wheelDelta);
  void updateScroll(float dt);
  bool isScrollUpActive() const { return m_scrollUpTimer > 0; }
  bool isScrollDownActive() const { return m_scrollDownTimer > 0; }

  int getScrollUpTimes() const { return m_scrollUpValue / 120; }
  int getScrollDownTimes() const { return m_scrollDownValue / -120; }

  bool isUpTimerBigger() const { return (m_scrollUpTimer > m_scrollDownTimer); }

  enum class ScrollDir { None, Up, Down };

  ScrollDir dominantScroll() const {
    if (m_scrollUpTimer <= 0.0f && m_scrollDownTimer <= 0.0f)
      return ScrollDir::None;
    return (m_scrollUpTimer > m_scrollDownTimer) ? ScrollDir::Up
                                                 : ScrollDir::Down;
  }
  bool isScrollingUp() const { return dominantScroll() == ScrollDir::Up; }
  bool isScrollingDown() const { return dominantScroll() == ScrollDir::Down; }

  float getTrailMinWidth() const { return m_trailMinWidth; }
  float getTrailMaxWidth() const { return m_trailMaxWidth; }
  RoundedRect getShape() const { return m_shape; }
  float getSmallRadius() const { return m_smallRadius; }
  // for Circle types
  float getCircleRadius() const { return m_circleRadius; }

  ImVec4 getOuterCol() const { return m_outerCol; }
  ImVec4 getInnerCol() const { return m_innerCol; }
  ImVec4 getFillCol() const { return m_fillCol; }
  ImVec4 getHighlightCol() const { return m_highlightCol; }
  ImVec4 getScrollCol() const { return m_scrollCol; }
  ImVec4 getTrailCol() const { return m_trailCol; }

private:
  void updateCatmullTrail(float deltaTime, const Config::MouseTheme &mouse,
                          float ovScale);
  void updateCircleTrail(float deltaTime, const Config::MouseTheme &mouse,
                         float ovScale);

  void ageTrail(float deltaTime, const Config::MouseTheme &mouse);
  void appendTrailPoint(const ImVec2 &pos, bool breakLine);
  void appendWrapSegment(const ImVec2 &current);
  void trimTrailToMaxLength(float maxLength);

  ImVec2 trailPos() const { return ImVec2{m_offsetX, m_offsetY}; }

  int m_pendingDeltaX{0};
  int m_pendingDeltaY{0};

  int m_deltaX{0};
  int m_deltaY{0};

  float m_offsetX{0.0f};
  float m_offsetY{0.0f};

  float m_sensitivity{0.5f};
  float m_decayRate{8.0f};

  float m_idleTime{0.0f};
  float m_fadeTime{0.6f};
  bool m_m1Highlighted{false};

  RoundedRect m_shape;
  bool m_justWrapped{false};
  ImVec2 m_wrapExit;

  float m_circleRadius{37.0f};
  float m_smallRadius{7.4f};

  std::deque<TrailPoint> m_trail;
  float m_trailTotalDist{0.0f};
  float m_trailMinWidth{1.0f};
  float m_trailMaxWidth{3.0f};

  int m_pendingScroll{0};
  int m_scrollUpValue{0};
  int m_scrollDownValue{0};
  float m_scrollDownTimer{0.0f};
  float m_scrollUpTimer{0.0f};

  ImVec4 m_outerCol{};
  ImVec4 m_innerCol{};
  ImVec4 m_fillCol{};
  ImVec4 m_highlightCol{};
  ImVec4 m_scrollCol{};
  ImVec4 m_trailCol{};
  bool m_colorsInited{false};
};

void renderMouseIndicator(const ImVec2 &center,
                          const Mouse::Type mouseStyleType,
                          const AppState &app);

bool isTypeCircle(Mouse::Type type);
bool isTypeRect(Mouse::Type type);

} // namespace Mouse

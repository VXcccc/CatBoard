#pragma once

#include "imgui.h"
#include <array>
#include <deque>
#include <string>
#include <windows.h>

namespace Config {

using ColorRGBA = std::array<int, 4>;
}

namespace Mouse {

enum class Type { CatBoard, NohBoard, Wrap, Pan, maxMouseStyles };

struct MouseStyle {
  std::string name{"Cat Board"};
  Type type{Type::CatBoard};
};

extern std::array<MouseStyle, static_cast<std::size_t>(Type::maxMouseStyles)>
    mouseStyles;

enum class TrailType { Circle, Catmull, maxTrailStyles };

struct TrailStyle {
  std::string name{"Catmull trail"};
  TrailType type{TrailType::Catmull};
};

extern std::array<TrailStyle,
                  static_cast<std::size_t>(TrailType::maxTrailStyles)>
    trailStyles;

struct TrailPoint {
  ImVec2 pos{};
  float age{0.0f};
  float distance{0.0f};
  bool breakLine{false};
  bool highlighted{false};
};

class MouseState {
public:
  MouseState(float sensitivity, float decayRate, float fadeTime)
      : m_sensitivity(sensitivity), m_decayRate(decayRate),
        m_fadeTime(fadeTime) {}

  // void setDeltaX(const POINT &currentMousePos, const POINT &prevMousePos) {
  //   m_deltaX = currentMousePos.x - prevMousePos.x;
  // }
  //
  // void setDeltaY(const POINT &currentMousePos, const POINT &prevMousePos) {
  //   m_deltaY = currentMousePos.y - prevMousePos.y;
  // }

  // void setOffsetX(float deltaX) { m_offsetX += deltaX * m_sensitivity; }
  float getOffsetX() const { return m_offsetX; }

  // void setOffsetY(float deltaY) { m_offsetY += deltaY * m_sensitivity; }
  float getOffsetY() const { return m_offsetY; }

  void setSensivity(float sensitivity) { m_sensitivity = sensitivity; }
  float getSensivity() const { return m_sensitivity; }

  void setDecayRate(float decayRate) { m_decayRate = decayRate; }
  float getDecayRate() const { return m_decayRate; }

  void updateMouseState(float deltaTime, float radius);
  void updateMouseState(float deltaTime, float halfWidth, float halfHeight);

  float getDistance() const;

  float getIdleTime() const { return m_idleTime; }
  float getFadeDur() const { return m_fadeTime; }

  void addRawDelta(int dx, int dy);
  void consumePendingDelta();

  void updateTrail(float deltaTime, TrailType type);
  const std::deque<TrailPoint> &getTrailPoints() const { return m_trail; }

  void registerScroll(int wheelDelta);
  void updateScroll(float dt);
  bool isScrollUpActive() const { return m_scrollUpTimer > 0; }
  bool isScrollDownActive() const { return m_scrollDownTimer > 0; }

  int getScrollUpTimes() const { return m_scrollUpValue / 120; }
  int getScrollDownTimes() const { return m_scrollDownValue / -120; }

  bool isUpTimerBigger() const { return (m_scrollUpTimer > m_scrollDownTimer); }

  // void addTotalTrailDist(float dist) { m_trailTotalDist += dist; }

private:
  void updateCatmullTrail(float deltaTime);
  void updateCircleTrail(float deltaTime);

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

  std::deque<TrailPoint> m_trail;
  float m_trailTotalDist{0.0f};

  int m_pendingScroll{0};
  int m_scrollUpValue{0};
  int m_scrollDownValue{0};
  float m_scrollDownTimer{0.0f};
  float m_scrollUpTimer{0.0f};

  bool m_justWrapped{false};

  bool m_panActive{false};
  ImVec2 trailPos() const { return ImVec2{m_offsetX, m_offsetY}; }
};

void renderMouseIndicator(const ImVec2 &center,
                          const Mouse::Type mouseStyleType);

void renderMouseTrail(const ImVec2 &center, TrailType type);

void updateM1Highlight(const std::array<bool, 256> &currentFullKeysDown);

bool isTypeCircle(Mouse::Type type);
bool isTypeRect(Mouse::Type type);

extern MouseState g_mouseState;
} // namespace Mouse

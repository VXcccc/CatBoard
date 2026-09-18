#pragma once

#include "imgui.h"
#include <string>
#include <vector>
#include <windows.h>

namespace Mouse {

enum class Type { CatBoard, NohBoard, maxMouseStyles };

struct MouseStyle {
  std::string name{"Cat Board"};
  Type type{Type::CatBoard};
};
extern std::array<MouseStyle, static_cast<std::size_t>(Type::maxMouseStyles)>
    mouseStyles;

class MouseState {
public:
  MouseState(float sensitivity, float decayRate)
      : m_sensitivity(sensitivity), m_decayRate(decayRate) {}

  void setDeltaX(const POINT &currentMousePos, const POINT &prevMousePos) {
    m_deltaX = currentMousePos.x - prevMousePos.x;
  }

  void setDeltaY(const POINT &currentMousePos, const POINT &prevMousePos) {
    m_deltaY = currentMousePos.y - prevMousePos.y;
  }

  // void setOffsetX(float deltaX) { m_offsetX += deltaX * m_sensitivity; }
  float getOffsetX() const { return m_offsetX; }

  // void setOffsetY(float deltaY) { m_offsetY += deltaY * m_sensitivity; }
  float getOffsetY() const { return m_offsetY; }

  void setSensivity(float sensitivity) { m_sensitivity = sensitivity; }
  float getSensivity() const { return m_sensitivity; }

  void setDecayRate(float decayRate) { m_decayRate = decayRate; }
  float getDecayRate() const { return m_decayRate; }

  void updateMouseState(float deltaTime, float radius);

  float getDistance() const;

private:
  int m_deltaX{0};
  int m_deltaY{0};

  float m_offsetX{0.0f};
  float m_offsetY{0.0f};

  float m_sensitivity{0.5f};
  float m_decayRate{8.0f};
};

void renderMouseIndicator(const ImVec2 &center,
                          const Mouse::Type mouseStyleType);

extern MouseState g_mouseState;
} // namespace Mouse

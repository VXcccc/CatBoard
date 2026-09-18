#include <MouseState.h>
#include <cmath>

namespace Mouse {

void MouseState::updateMouseState(float deltaTime, float radius) {

  m_offsetX += m_deltaX * m_sensitivity;
  m_offsetY += m_deltaY * m_sensitivity;

  float decay{std::exp(-m_decayRate * deltaTime)};
  m_offsetX *= decay;
  m_offsetY *= decay;

  float dist{std::sqrt(m_offsetX * m_offsetX + m_offsetY * m_offsetY)};

  if (dist > radius) {
    float scale{radius / dist};
    m_offsetX *= scale;
    m_offsetY *= scale;
  }
}

} // namespace Mouse

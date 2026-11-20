#pragma once

#include "imgui.h"
#include "wooting-analog-sdk.h"
#include <cmath>
#include <string>

namespace Analog {
std::string getAnalogResult(WootingAnalogResult res);

bool init();
bool isInited();

float readKey(unsigned short vk);

bool shutdown();

enum class AnimType { Up, Down, MaxTypes };

struct State {
  ImVec4 color{};
  float value{};

  float getCutY(float startY, float targetY) const {
    return std::lerp(startY, targetY, value);
  }
};

} // namespace Analog

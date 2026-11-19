#pragma once

#include "Layouts.h"
#include "imgui.h"
#include "wooting-analog-sdk.h"
#include <cmath>
#include <iostream>
#include <string>
#include <utility>

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

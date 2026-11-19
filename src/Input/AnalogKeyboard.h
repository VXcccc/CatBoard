#pragma once

#include "Layouts.h"
#include "imgui.h"
#include "wooting-analog-sdk.h"
#include <iostream>
#include <string>
#include <utility>

namespace Analog {
std::string getAnalogResult(WootingAnalogResult res);

bool init();
bool isInited();

float readKey(unsigned short vk);

bool shutdown();

enum class AnimationType { Up, Down, MaxTypes };

struct State {
  AnimationType animType{AnimationType::Up};
  ImVec4 color{};
  float target{};
  float drawY{};
  std::pair<float, float> getTargets(const Layouts::Key &key) {
    switch (animType) {

    case AnimationType::Up: {
      float target = key.y;
      float drawY = key.y + key.h;
      return {target, drawY};
    }
    case AnimationType::Down: {
      float target = key.y + key.h;
      float drawY = key.y;
      return {target, drawY};
    }

    case AnimationType::MaxTypes:
      break;
    }
    return {0.0f, 0.0f};
  }
  std::pair<ImVec2, ImVec2> drawAnimation(const Layouts::Key &key) const {
    switch (animType) {
    case AnimationType::Up: {
      ImVec2 topLeft(key.x, drawY);
      ImVec2 bottomRight(key.x + key.w, key.y + key.h);
      return {topLeft, bottomRight};
    }
    case AnimationType::Down: {
      ImVec2 topLeft(key.x, key.y);
      ImVec2 bottomRight(key.x + key.w, drawY);
      return {topLeft, bottomRight};
    }
    case AnimationType::MaxTypes:
      break;
    }
    return std::pair<ImVec2, ImVec2>({0.0f, 0.0f}, {0.0f, 0.0f});
  }
};

// class Keyboard{
// 	public:
// 		Keyboard() : {};
//
// 	private:
//
// };

} // namespace Analog

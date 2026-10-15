#include "Keyboard.h"
#include "Config/Config.h"
#include "Media/Sound.h"
#include "System/Helpers.h"
#include "UI/Layouts.h"
#include "UI/Render.h"
#include "imgui.h"
#include <array>
#include <utility>
#include <windows.h>

namespace Keyboard {

static std::pair<ImVec2, ImVec2>
getCenterAndHalfSize(const Layouts::Key &key, float scale,
                     Config::AnimType animType) {
  switch (animType) {
  case Config::AnimType::Lift:
    return {{key.x + key.w * 0.5f, key.y + (key.h * scale) * 0.5f},
            {key.w * 0.5f, key.h * 0.5f}};
  default:
    return {{key.x + key.w * 0.5f, key.y + key.h * 0.5f},
            {key.w * scale * 0.5f, key.h * scale * 0.5f}};
  }
}

void renderKeyboard(const Layouts::Layout &layout,
                    const std::array<bool, 256> &currentKeysDown,
                    const std::array<bool, 256> &currentFullKeysDown,
                    const std::array<float, 256> &keysScale,
                    const std::array<Render::KeyColors, 256>
                        &keysColors) { // todo maybe remove currentFullKeysDown
  ImDrawList *draw{ImGui::GetBackgroundDrawList()};
  const auto &theme{Config::activeConfig.theme};
  ImGui::PushFont(Fonts::overlayFont);

  if (theme.keysImagesDisplay) {
    Images::drawKeyboardImages(draw, currentKeysDown);
  }

  for (const auto &key : Layouts::activeLayout.keys) {
    bool down{currentKeysDown[key.keyCode]};
    // bool down{(GetAsyncKeyState(key.keyCode) & 0x8000) != 0};

    // i did first if statement for peoples who have caps binded to smth, you
    // can remove it if you want to show caps lock toogle state without
    // changing buttons labels
    if (Config::activeConfig.font.letterCase.options.capsLockDisplay) {
      if (key.keyCode == VK_CAPITAL) {
        down = (GetKeyState(VK_CAPITAL) & 1) != 0;
      }
    }

    const float scale{keysScale[key.keyCode]};
    std::pair<ImVec2, ImVec2> keyCenterHalfSize{
        getCenterAndHalfSize(key, scale, theme.animType)};

    const ImVec2 center{keyCenterHalfSize.first};
    const ImVec2 halfSize{keyCenterHalfSize.second};

    const auto &ovScale{Config::activeConfig.overlay.scale};
    ImVec2 topLeft((center.x - halfSize.x) * ovScale,
                   (center.y - halfSize.y) * ovScale);
    ImVec2 bottomRight((center.x + halfSize.x) * ovScale,
                       (center.y + halfSize.y) * ovScale);

    ImU32 fill{
        Helper::ToImU32(Helper::toConfigColor(keysColors[key.keyCode].fill))};

    ImU32 textColor{
        Helper::ToImU32(Helper::toConfigColor(keysColors[key.keyCode].text))};

    float rounding{theme.keyRounding};

    draw->AddRectFilled(topLeft, bottomRight, fill, rounding);

    if (theme.keyOutline.enable) {
      ImU32 outlineColor{Helper::ToImU32(
          Helper::toConfigColor(keysColors[key.keyCode].outline))};
      float thickness{theme.keyOutline.thickness};
      draw->AddRect(topLeft, bottomRight, outlineColor, rounding, 0, thickness);
    }

    std::string displayLabel{Fonts::handleLetterCase(
        currentFullKeysDown, Config::activeConfig.font, key)};

    ImVec2 textSize{ImGui::CalcTextSize(key.label.c_str())};
    ImVec2 textPos{(center.x * ovScale) - (textSize.x * 0.5f) ,
                   (center.y * ovScale) - (textSize.y * 0.5f) };
    draw->AddText(textPos, textColor, displayLabel.c_str());
  }
  ImGui::PopFont();
}

static ImVec4 animateColor(ImVec4 &current, const ImVec4 &target, float speed,
                           float deltaTime) {
  float t{1.0f - std::exp(-speed * deltaTime)};

  current.x += (target.x - current.x) * t;
  current.y += (target.y - current.y) * t;
  current.z += (target.z - current.z) * t;
  current.w += (target.w - current.w) * t;

  return current;
}

bool detectAnyKeyChanged(const Layouts::Layout &layout,
                         const std::array<bool, 256> &prevKeyState,
                         std::array<bool, 256> &outCurrentKeysDown) {
  bool anyKeyChanged = false;
  for (const auto &key : layout.keys) {
    bool down{(GetAsyncKeyState(key.keyCode) & 0x8000) != 0};
    outCurrentKeysDown[key.keyCode] = down;
    if (down != prevKeyState[key.keyCode]) {
      anyKeyChanged = true;
    }
  }

  return anyKeyChanged;
}

bool detectAnyMouseKeyChanged(const std::array<bool, 256> &prevKeyState,
                              std::array<bool, 256> &outCurrentKeysDown) {
  constexpr std::array<int, 5> mouseKeys = {VK_LBUTTON, VK_RBUTTON, VK_MBUTTON,
                                            VK_XBUTTON1, VK_XBUTTON2};
  bool anyKeyChanged = false;
  for (int keyCode : mouseKeys) {
    bool down = (GetAsyncKeyState(keyCode) & 0x8000) != 0;

    outCurrentKeysDown[keyCode] = down;

    if (down != prevKeyState[keyCode]) {
      anyKeyChanged = true;
    }
  }
  return anyKeyChanged;
}

void pollFullKeyboard(std::array<bool, 256> &outCurrentKeysDown) {
  for (int vk{0}; vk < 256; ++vk) {
    outCurrentKeysDown[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
  }
}

void commitKeyState(const Layouts::Layout &layout,
                    const std::array<bool, 256> &currentKeysDown,
                    std::array<bool, 256> &outPrevKeyState) {
  for (const auto &key : layout.keys) {
    outPrevKeyState[key.keyCode] = currentKeysDown[key.keyCode];
  }
}

// sound functions
void handleKeySound(const Layouts::Layout &layout,
                    const std::array<bool, 256> &prevKeyState,
                    const std::array<bool, 256> &currentKeysDown) {
  for (const auto &key : layout.keys) {
    bool down{currentKeysDown[key.keyCode]};
    if (down != prevKeyState[key.keyCode]) {
      Sound::playKey(key.keyCode, down);
    }
  }
}

void handleKeySoundFull(const std::array<bool, 256> &prevKeyState,
                        const std::array<bool, 256> &currentKeysDown) {
  for (const auto &[vk, def] : Sound::activePack.KeyDefByVK) {
    bool down{currentKeysDown[vk]};
    if (down != prevKeyState[vk]) {
      Sound::playKey(vk, down);
    }
  }
}

// animations

bool updateKeyColors(const Layouts::Layout &layout,
                     const std::array<bool, 256> &currentKeysDown,
                     std::array<Render::KeyColors, 256> &outColors,
                     float deltaTime) {
  const auto &theme{Config::activeConfig.theme};
  const float speed{static_cast<float>(theme.animationSpeed) / 10.0f};
  bool animationActive{false};

  for (const auto &key : layout.keys) {
    bool down{currentKeysDown[key.keyCode]};

    ImVec4 fill{
        Helper::fromConfigColor(down ? theme.activeFill : theme.idleFill)};
    ImVec4 text{
        Helper::fromConfigColor(down ? theme.activeText : theme.idleText)};
    ImVec4 outline{Helper::fromConfigColor(down ? theme.keyOutline.activeColor
                                                : theme.keyOutline.idleColor)};

    if (theme.animateColors && theme.animation) {
      outColors[key.keyCode].fill =
          animateColor(outColors[key.keyCode].fill, fill, speed, deltaTime);
      outColors[key.keyCode].text =
          animateColor(outColors[key.keyCode].text, text, speed, deltaTime);
      outColors[key.keyCode].outline = animateColor(
          outColors[key.keyCode].outline, outline, speed, deltaTime);
      animationActive = true;
    } else {
      outColors[key.keyCode].fill = fill;
      outColors[key.keyCode].text = text;
      outColors[key.keyCode].outline = outline;
      animationActive = false;
    }
  }
  return animationActive;
}

bool updateAnimations(const Layouts::Layout &layout,
                      const std::array<bool, 256> &currentKeysDown,
                      std::array<float, 256> &outKeysScale, float deltaTime) {
  const auto &theme{Config::activeConfig.theme};

  static bool prevAnimation{true};
  if (!theme.animation || theme.animationSpeed <= 0) {
    if (prevAnimation) {
      for (const auto &key : layout.keys)
        outKeysScale[key.keyCode] = 1.0f;
      prevAnimation = false;
      return true;
    }
    return false;
  }

  prevAnimation = true;
  bool animationActive{false};

  const float speed{static_cast<float>(theme.animationSpeed) / 10.0f};
  const float t{1.0f - std::exp(-speed * deltaTime)};

  for (const auto &key : layout.keys) {

    bool down{currentKeysDown[key.keyCode]};
    float targetScale{(down) ? theme.pressScale : 1.0f};

    outKeysScale[key.keyCode] =
        std::lerp(outKeysScale[key.keyCode], targetScale, t);

    if (std::abs(outKeysScale[key.keyCode] - 1.0f) > 0.001f) {
      animationActive = true;
    }
  }
  return animationActive;
}

} // namespace Keyboard

#include "Keyboard.h"
#include "Config/Config.h"
#include "Input/Mouse.h"
#include "Media/Sound.h"
#include "System/Helpers.h"
#include "UI/Layouts.h"
#include "imgui.h"
#include <cmath>
#include <utility>
#include <windows.h>

namespace Keyboard {

void Keyboard::State::pollFullKeyboard() {
  prevDown = down;
  for (int vk{0}; vk < 256; ++vk) {
    down[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
  }
}

void Keyboard::State::resyncVisuals(const Layouts::Layout &layout) {
  const auto &theme{Config::activeConfig.theme};
  visuals.assign(layout.keys.size(), KeyVisual{});
  for (auto &v : visuals) {
    v.colors.fill = Helper::fromConfigColor(theme.idleFill);
    v.colors.text = Helper::fromConfigColor(theme.idleText);
    v.colors.outline = Helper::fromConfigColor(theme.keyOutline.idleColor);
  }
}

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

bool isActive(const Layouts::Key &key, const Keyboard::State &kb,
              const Mouse::MouseState &m) {
  switch (key.type) {
  case Layouts::KeyType::Keyboard:
    return kb.down[key.code];
  case Layouts::KeyType::Wheel:
    return (kb.down[VK_MBUTTON] || m.isScrollDownActive() ||
            m.isScrollUpActive());
  case Layouts::KeyType::ScrollUpDown:
    return (m.isScrollUpActive() || m.isScrollDownActive());
  case Layouts::KeyType::ScrollUp:
    return (m.isScrollUpActive());
  case Layouts::KeyType::ScrollDown:
    return (m.isScrollDownActive());
  default:
    break;
  }
  return false;
}

void updateActive(const Layouts::Layout &layout, Keyboard::State &kb,
                  const Mouse::MouseState &mouse) {
  if (kb.visuals.size() != layout.keys.size())
    kb.resyncVisuals(layout);
  for (std::size_t i = 0; i < layout.keys.size(); ++i)
    kb.visuals[i].active = isActive(layout.keys[i], kb, mouse);
}

void renderKeyboard(const Layouts::Layout &layout, const Keyboard::State &kb) {
  ImDrawList *draw{ImGui::GetBackgroundDrawList()};
  const auto &theme{Config::activeConfig.theme};
  ImGui::PushFont(Fonts::overlayFont);

  if (theme.keysImagesDisplay) {
    Images::drawKeyboardImages(draw, kb);
  }

  for (std::size_t i{0}; i < layout.keys.size(); ++i) {
    const auto &v{kb.visuals[i]};
    const auto &key{layout.keys[i]};
    bool down{v.active};

    // i did first if statement for peoples who have caps binded to smth, you
    // can remove this if statement if you wanna show caps lock toogle kb
    // without changing buttons labels
    if (Config::activeConfig.font.letterCase.options.capsLockDisplay) {
      if (key.code == VK_CAPITAL) {
        down = (GetKeyState(VK_CAPITAL) & 1) != 0;
      }
    }

    const float scale{v.scale};
    std::pair<ImVec2, ImVec2> keyCenterHalfSize{
        getCenterAndHalfSize(key, scale, theme.animType)};

    const ImVec2 center{keyCenterHalfSize.first};
    const ImVec2 halfSize{keyCenterHalfSize.second};

    const auto &ovScale{Config::activeConfig.overlay.scale};
    ImVec2 topLeft((center.x - halfSize.x) * ovScale,
                   (center.y - halfSize.y) * ovScale);
    ImVec2 bottomRight((center.x + halfSize.x) * ovScale,
                       (center.y + halfSize.y) * ovScale);

    ImU32 fill{Helper::ToImU32(Helper::toConfigColor(v.colors.fill))};

    ImU32 textColor{Helper::ToImU32(Helper::toConfigColor(v.colors.text))};

    float rounding{theme.keyRounding};

    draw->AddRectFilled(topLeft, bottomRight, fill, rounding);

    if (theme.keyOutline.enable) {
      ImU32 outlineColor{
          Helper::ToImU32(Helper::toConfigColor(v.colors.outline))};
      float thickness{theme.keyOutline.thickness};
      draw->AddRect(topLeft, bottomRight, outlineColor, rounding, 0, thickness);
    }

    std::string displayLabel{
        Fonts::handleLetterCase(kb.down, Config::activeConfig.font, key)};

    ImVec2 textSize{ImGui::CalcTextSize(key.label.c_str())};
    ImVec2 textPos{(center.x * ovScale) - (textSize.x * 0.5f),
                   (center.y * ovScale) - (textSize.y * 0.5f)};
    draw->AddText(textPos, textColor, displayLabel.c_str());
  }
  ImGui::PopFont();
}

// bool detectAnyMouseKeyChanged(Keyboard::State &kb) {
//   constexpr std::array<int, 5> mouseKeys = {VK_LBUTTON, VK_RBUTTON,
//   VK_MBUTTON,
//                                             VK_XBUTTON1, VK_XBUTTON2};
//   bool anyKeyChanged{false};
//   for (int keyCode : mouseKeys) {
//     bool down{kb.down[keyCode]};
//     if (down != kb.prevDown[keyCode]) {
//       anyKeyChanged = true;
//     }
//   }
//   return anyKeyChanged;
// }

// sound functions
void handleKeySound(const Layouts::Layout &layout, const Keyboard::State &kb) {
  for (const auto &key : layout.keys) {
    bool down{kb.down[key.code]};
    if (down != kb.prevDown[key.code]) {
      Sound::playKey(key.code, down);
    }
  }
}

void handleKeySoundFull(const Keyboard::State &kb) {
  for (const auto &[vk, def] : Sound::activePack.KeyDefByVK) {
    bool down{kb.down[vk]};
    if (down != kb.prevDown[vk]) {
      Sound::playKey(vk, down);
    }
  }
}

// animations
constexpr float minAnimStep = 0.002f;

// basically lerping the color or scale
static bool animate(float &cur, float target, float t) {
  cur = std::lerp(cur, target, t);
  if (std::abs(target - cur) < minAnimStep) {
    cur = target;
    return false;
  }
  return true;
}

static bool animateColor(ImVec4 &cur, const ImVec4 &target, float t) {
  bool moving = false;
  moving |= animate(cur.x, target.x, t);
  moving |= animate(cur.y, target.y, t);
  moving |= animate(cur.z, target.z, t);
  moving |= animate(cur.w, target.w, t);
  return moving;
}

bool updateKeyColors(const Layouts::Layout &layout, Keyboard::State &kb,
                     float deltaTime) {
  const auto &theme{Config::activeConfig.theme};
  const bool animEnabled{theme.animateColors && theme.animation &&
                         theme.animationSpeed > 0};
  const float speed{static_cast<float>(theme.animationSpeed) / 10.0f};
  float t{1.0f - std::exp(-speed * deltaTime)};
  bool animationActive{false};

  for (std::size_t i{0}; i < layout.keys.size(); ++i) {
    // const auto &key{layout.keys[i]};
    auto &v{kb.visuals[i]};
    bool down{v.active};

    ImVec4 fill{
        Helper::fromConfigColor(down ? theme.activeFill : theme.idleFill)};
    ImVec4 text{
        Helper::fromConfigColor(down ? theme.activeText : theme.idleText)};
    ImVec4 outline{Helper::fromConfigColor(down ? theme.keyOutline.activeColor
                                                : theme.keyOutline.idleColor)};

    if (!animEnabled) {
      v.colors.fill = fill;
      v.colors.text = text;
      v.colors.outline = outline;
      continue;
    }
    animationActive |= animateColor(v.colors.fill, fill, t);
    animationActive |= animateColor(v.colors.text, text, t);
    animationActive |= animateColor(v.colors.outline, outline, t);
  }
  return animationActive;
}

bool updateScale(const Layouts::Layout &layout, Keyboard::State &kb,
                 float deltaTime) {
  const auto &theme{Config::activeConfig.theme};
  const bool animEnabled{theme.animation && theme.animationSpeed > 0};
  const float speed{static_cast<float>(theme.animationSpeed) / 10.0f};
  float t{1.0f - std::exp(-speed * deltaTime)};
  bool animationActive{false};

  for (std::size_t i{0}; i < layout.keys.size(); ++i) {
    auto &v{kb.visuals[i]};
    bool down{v.active};

    if (!animEnabled) {
      v.scale = 1.0f;
      continue;
    }

    float targetScale{(down) ? theme.pressScale : 1.0f};
    animationActive |= animate(v.scale, targetScale, t);
  }
  return animationActive;
}
} // namespace Keyboard

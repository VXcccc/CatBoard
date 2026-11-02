#include "Keyboard.h"
#include "AppState.h"
#include "Config.h"
#include "Fonts.h"
#include "Helpers.h"
#include "Layouts.h"
#include "Mouse.h"
#include "Sound.h"
#include "imgui.h"
#include <cmath>
#include <cstdio>
#include <string>
#include <utility>
#include <windows.h>

namespace Keyboard {

void Keyboard::State::pollFullKeyboard() {
  prevDown = down;
  for (int vk{0}; vk < 256; ++vk) {
    down[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
  }
}

void Keyboard::State::resyncVisuals(const Layouts::Layout &layout,
                                    const Config::Theme &theme) {
  visuals.assign(layout.keys.size(), KeyVisual{});
  for (auto &v : visuals) {
    v.colors.fill = Helper::fromConfigColor(theme.idleFill);
    v.colors.text = Helper::fromConfigColor(theme.idleText);
    v.colors.outline = Helper::fromConfigColor(theme.keyOutline.idleColor);
  }
}

bool isActive(const Layouts::Key &key, const Keyboard::State &kb,
              const Mouse::State &m, const Config::Settings &config) {
  const bool capsDisplay{config.font.letterCase.options.capsLockDisplay};

  switch (key.type) {
  case Layouts::KeyType::Keyboard:
    // caps handle
    if (key.code == VK_CAPITAL && capsDisplay)
      return GetAsyncKeyState(VK_CAPITAL) & 1;
    // other keys
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
                  const Mouse::State &mouse,
                  const Config::Settings &settings) {
  if (kb.visuals.size() != layout.keys.size())
    kb.resyncVisuals(layout, settings.theme);
  for (std::size_t i = 0; i < layout.keys.size(); ++i)
    kb.visuals[i].active = isActive(layout.keys[i], kb, mouse, settings);
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

static std::string getScrollLabel(Mouse::State::ScrollDir dir,
                                  const Layouts::Key &key,
                                  const Mouse::State &mouse) {
  using type = Mouse::State::ScrollDir;
  using kType = Layouts::KeyType;

  switch (dir) {
  case type::None:
    return key.label;
  case type::Up:
    if (key.type == kType::ScrollDown)
      return key.label;
    return std::to_string(mouse.getScrollUpTimes());
  case type::Down:
    if (key.type == kType::ScrollUp)
      return key.label;
    return std::to_string(mouse.getScrollDownTimes());
  }
  return key.label;
}

static std::string getScrollIcon(Mouse::State::ScrollDir dir,
                                 const Layouts::Key &key) {
  using type = Mouse::State::ScrollDir;
  using kType = Layouts::KeyType;

  switch (dir) {
  case type::Up:
    if (key.type == kType::ScrollDown)
      return {""};
    return std::string{Icons::arrowUp};
  case type::Down:
    if (key.type == kType::ScrollUp)
      return {""};
    return std::string{Icons::arrowDown};
  default:
    return {""};
  }
}

static std::string getDisplayLabel(const Layouts::Key &key,
                                   const Keyboard::State &kb, Fonts::Font &font,
                                   const Mouse::State &mouse) {

  if (key.type == Layouts::KeyType::Keyboard) {
    return Fonts::handleLetterCase(kb.down, font, key);
  } else {
    return getScrollLabel(mouse.dominantScroll(), key, mouse);
  }
}

static void drawText(ImDrawList *draw, const Layouts::Key &key,
                     const Keyboard::State &kb, const KeyVisual &v,
                     ImVec2 center, Config::Settings &settings,
                     const Mouse::State &mouse) {
  ImGui::PushFont(Fonts::overlayFont);

  using ScrollDir = Mouse::State::ScrollDir;

  const ImU32 textColor{Helper::ToImU32(Helper::toConfigColor(v.colors.text))};
  const float &ovScale{settings.overlay.scale};

  std::string displayLabel{getDisplayLabel(key, kb, settings.font, mouse)};

  ImVec2 textSize{ImGui::CalcTextSize(displayLabel.c_str())};
  ImVec2 textPos{(center.x * ovScale) - (textSize.x * 0.5f),
                 (center.y * ovScale) - (textSize.y * 0.5f)};

  draw->AddText(textPos, textColor, displayLabel.c_str());
  ImGui::PopFont();

  // draw icons for special keys
  if (!(key.type == Layouts::KeyType::Keyboard)) {
    const auto curDir(mouse.dominantScroll());

    if (!(curDir == ScrollDir::None)) {
      ImGui::PushFont(Fonts::iconFont);
      const std::string curIcon{getScrollIcon(curDir, key)};
      const ImVec2 iconSize{ImGui::CalcTextSize(curIcon.c_str())};
      printf("%s iconSize: x: %.2f, y: %.2f ", curIcon.c_str(), textSize.x,
             iconSize.y);
      printf("textPos: x: %.2f, y: %.2f\n", textPos.x, textPos.y);
      const ImVec2 iconPos{(curDir == ScrollDir::Up ? (textPos.x + textSize.x)
                                                    : (textPos.x - iconSize.x)),
                           (center.y - (iconSize.y * 0.5f))};

      draw->AddText(iconPos, textColor, curIcon.c_str());
      ImGui::PopFont();
    }
  }
}

void renderKeyboard( AppState &app) {
  ImDrawList *draw{ImGui::GetBackgroundDrawList()};
  const auto &layout{app.layout};
  const auto &kb{app.keyboard};
  const auto &theme{app.config.theme};

  if (theme.keysImagesDisplay) {
    Images::drawKeyboardImages(draw, app);
  }

  for (std::size_t i{0}; i < layout.keys.size(); ++i) {
    const auto &v{kb.visuals[i]};
    const auto &key{layout.keys[i]};
    bool down{v.active};

    const float scale{v.scale};
    std::pair<ImVec2, ImVec2> keyCenterHalfSize{
        getCenterAndHalfSize(key, scale, theme.animType)};

    const ImVec2 center{keyCenterHalfSize.first};
    const ImVec2 halfSize{keyCenterHalfSize.second};

    const auto &ovScale{app.config.overlay.scale};
    ImVec2 topLeft((center.x - halfSize.x) * ovScale,
                   (center.y - halfSize.y) * ovScale);
    ImVec2 bottomRight((center.x + halfSize.x) * ovScale,
                       (center.y + halfSize.y) * ovScale);

    ImU32 fill{Helper::ToImU32(Helper::toConfigColor(v.colors.fill))};

    float rounding{theme.keyRounding};

    draw->AddRectFilled(topLeft, bottomRight, fill, rounding);

    if (theme.keyOutline.enable) {
      ImU32 outlineColor{
          Helper::ToImU32(Helper::toConfigColor(v.colors.outline))};
      float thickness{theme.keyOutline.thickness};
      draw->AddRect(topLeft, bottomRight, outlineColor, rounding, 0, thickness);
    }
    drawText(draw, key, kb, v, center, app.config, app.mouse);
  }
}

// sound functions
static void handleKeySoundLayout(const Layouts::Layout &layout,
                                 const Keyboard::State &kb,
                                 const Sound::PackVK &soundPack) {
  for (const auto &key : layout.keys) {
    bool down{kb.down[key.code]};
    if (down != kb.prevDown[key.code]) {
      Sound::playKey(key.code, down, soundPack);
    }
  }
}

static void handleKeySoundFull(const Keyboard::State &kb,
                               const Sound::PackVK &soundPack) {
  for (const auto &key : soundPack.KeyDefByVK) {
    const auto &vk = key.first;
    bool down{kb.down[vk]};
    if (down != kb.prevDown[vk]) {
      Sound::playKey(vk, down, soundPack);
    }
  }
}

void hadnleKeySounds(const Keyboard::State &kb, bool layoutKeysOnly,
                     const Layouts::Layout &layout,
                     const Sound::PackVK &soundPack) {
  if (!kb.anyChanged())
    return;

  if (layoutKeysOnly) {
    handleKeySoundLayout(layout, kb, soundPack);
  } else {
    handleKeySoundFull(kb, soundPack);
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
                     float deltaTime, const Config::Theme &theme) {
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
                 float deltaTime, const Config::Theme &theme) {
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

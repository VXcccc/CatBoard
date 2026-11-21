#include "Keyboard.h"
#include "AnalogKeyboard.h"
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

// raw value
bool isKeyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

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

bool Keyboard::State::anyKeyHeld(const Layouts::Layout &layout) const {
  bool anyKeyHeld{false};
  for (std::size_t i{0}; i < layout.keys.size(); ++i) {
    auto &v{visuals[i]};
    auto &key{layout.keys[i]};
    if (v.active || v.analog.value > 0.0f) {
      anyKeyHeld = true;
    }
  }
  return anyKeyHeld;
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
                  const Mouse::State &mouse, const Config::Settings &config) {
  if (kb.visuals.size() != layout.keys.size())
    kb.resyncVisuals(layout, config.theme);
  for (std::size_t i = 0; i < layout.keys.size(); ++i) {
    kb.visuals[i].active = isActive(layout.keys[i], kb, mouse, config);
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
                     const Keyboard::State &kb, float vScale, ImVec2 center,
                     Config::Settings &config, const Mouse::State &mouse,
                     const ImVec4 &color) {
  const auto &theme{config.theme};
  const auto &imStyle{ImGui::GetStyle()};
  const float &ovScale{config.overlay.scale};

  const bool applyScaleToFont{vScale > 0.01f && theme.animation &&
                              theme.animType == Config::AnimType::Scale};
  const float fontSizePx{applyScaleToFont
                             ? config.font.sizePixels * vScale * ovScale
                             : config.font.sizePixels * ovScale};
  ImGui::PushFont(Fonts::overlayFont, fontSizePx);

  using ScrollDir = Mouse::State::ScrollDir;

  const ImU32 textColor{Helper::ToImU32(Helper::toConfigColor(color))};

  std::string displayLabel{getDisplayLabel(key, kb, config.font, mouse)};

  ImVec2 textSize{ImGui::CalcTextSize(displayLabel.c_str())};
  ImVec2 textPos{(center.x * ovScale) - (textSize.x * 0.5f),
                 (center.y * ovScale) - (textSize.y * 0.5f)};

  draw->AddText(textPos, textColor, displayLabel.c_str());
  ImGui::PopFont();

  // draw icons for special keys
  if (!(key.type == Layouts::KeyType::Keyboard)) {
    const auto curDir(mouse.dominantScroll());

    if (!(curDir == ScrollDir::None)) {
      ImGui::PushFont(Fonts::iconFont, fontSizePx);
      const std::string curIcon{getScrollIcon(curDir, key)};
      const ImVec2 iconSize{ImGui::CalcTextSize(curIcon.c_str())};
      // printf("%s iconSize: x: %.2f, y: %.2f ", curIcon.c_str(), textSize.x,
      //        iconSize.y);
      // printf("textPos: x: %.2f, y: %.2f\n", textPos.x, textPos.y);
      const ImVec2 iconPos{(curDir == ScrollDir::Up ? (textPos.x + textSize.x)
                                                    : (textPos.x - iconSize.x)),
                           ((center.y * ovScale) - (iconSize.y * 0.5f))};

      draw->AddText(iconPos, textColor, curIcon.c_str());
      ImGui::PopFont();
    }
  }
}

static void updateFontSize(const Layouts::Key &key, const KeyVisual &v) {
  auto style{ImGui::GetStyle()};
  style.FontScaleMain = v.scale;
  style.FontSizeBase *= v.scale;
}

static bool isAnalogAnimUp(const Analog::AnimType animType) {
  return animType == Analog::AnimType::Up;
}

static void drawAnalogKey(ImDrawList *draw, const KeyVisual &v,
                          const Config::Theme &theme, const ImVec2 &topLeft,
                          const ImVec2 &bottomRight, float ovScale,
                          float rounding) {
  if (!theme.analog.enabled)
    return;

  const bool up{isAnalogAnimUp(theme.analog.animType)};

  const float cutY{up ? v.analog.getCutY(bottomRight.y, topLeft.y)
                      : v.analog.getCutY(topLeft.y, bottomRight.y)};

  const ImVec2 clipMin{topLeft.x, up ? cutY : topLeft.y};
  const ImVec2 clipMax{bottomRight.x, up ? bottomRight.y : cutY};

  const ImU32 color{v.active ? Helper::ToImU32(theme.analog.pressedColor)
                             : Helper::ToImU32(theme.analog.color)};

  if (clipMax.y > clipMin.y) {
    draw->PushClipRect(clipMin, clipMax, true);
    draw->AddRectFilled(topLeft, bottomRight, color, rounding);
    draw->PopClipRect();
  }
}

// basically applying hall effect to the text using active and idletext
static void drawAnalogText(ImDrawList *draw, const Layouts::Key &key,
                           const Keyboard::State &kb, const KeyVisual &v,
                           ImVec2 center, Config::Settings &config,
                           const Mouse::State &mouse, const ImVec2 &topLeft,
                           const ImVec2 &bottomRight) {
  const auto &theme{config.theme};
  const float &ovScale{config.overlay.scale};

  const bool up{isAnalogAnimUp(theme.analog.animType)};

  const float cutY{up ? v.analog.getCutY(topLeft.y, bottomRight.y)
                      : v.analog.getCutY(bottomRight.y, topLeft.y)};
  constexpr float big{100000.0f};

  const ImVec2 coveredMin{-big, up ? cutY : -big};
  const ImVec2 coveredMax{big, up ? big : cutY};

  const ImVec2 freeMin{-big, up ? -big : cutY};
  const ImVec2 freeMax{big, up ? cutY : big};

  draw->PushClipRect(freeMin, freeMax, true);
  drawText(draw, key, kb, v.scale, center, config, mouse,
           Helper::fromConfigColor(theme.idleText));
  draw->PopClipRect();

  draw->PushClipRect(coveredMin, coveredMax, true);
  drawText(draw, key, kb, v.scale, center, config, mouse,
           Helper::fromConfigColor(theme.activeText));
  draw->PopClipRect();
}

void renderKeyboard(AppState &app) {
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
    const std::pair<ImVec2, ImVec2> keyCenterHalfSize{
        getCenterAndHalfSize(key, scale, theme.animType)};

    const ImVec2 center{keyCenterHalfSize.first};
    const ImVec2 halfSize{keyCenterHalfSize.second};

    const auto &ovScale{app.config.overlay.scale};
    ImVec2 topLeft((center.x - halfSize.x) * ovScale,
                   (center.y - halfSize.y) * ovScale);
    ImVec2 bottomRight((center.x + halfSize.x) * ovScale,
                       (center.y + halfSize.y) * ovScale);

    ImU32 fill{Helper::ToImU32(Helper::toConfigColor(v.colors.fill))};

    float rounding{theme.keyRounding * ovScale};

    draw->AddRectFilled(topLeft, bottomRight, fill, rounding);

    if (theme.analog.enabled) {
      drawAnalogKey(draw, v, theme, topLeft, bottomRight, ovScale, rounding);
    }

    if (theme.keyOutline.enable) {
      ImU32 outlineColor{
          Helper::ToImU32(Helper::toConfigColor(v.colors.outline))};
      float thickness{theme.keyOutline.thickness};
      draw->AddRect(topLeft, bottomRight, outlineColor, rounding, 0, thickness);
    }
    // updateFontSize(key, v);
    if (theme.analog.enabled && v.analog.value > 0.0f &&
        theme.analog.enableHallText) {
      drawAnalogText(draw, key, kb, v, center, app.config, app.mouse,
                     bottomRight, topLeft);
    } else {
      drawText(draw, key, kb, v.scale, center, app.config, app.mouse,
               v.colors.text);
    }
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

static bool animateKeyColor(ImVec4 &cur, const ImVec4 &target, bool hide,
                            float colorT, float hideT) {
  const ImVec4 rgbTarget{target.x, target.y, target.z, cur.w};
  bool moving{Helper::animateColor(cur, rgbTarget, colorT)};

  moving |= Helper::animate(cur.w, hide ? 0.0f : target.w, hideT);
  return moving;
}

bool updateKeyColors(const Layouts::Layout &layout, Keyboard::State &kb,
                     float deltaTime, const Config::Theme &theme,
                     bool hideOverlay, int hideAnimSpeed) {
  const bool animEnabled{theme.animateColors && theme.animation &&
                         theme.animationSpeed > 0};
  const float speed{static_cast<float>(theme.animationSpeed) / 10.0f};
  const float hideSpeed{static_cast<float>(hideAnimSpeed) / 10.0f};

  const float colorT{animEnabled ? 1.0f - std::exp(-speed * deltaTime) : 1.0f};
  const float hideT{1.0f - std::exp(-hideSpeed * deltaTime)};

  bool animationActive{false};

  for (std::size_t i{0}; i < layout.keys.size(); ++i) {
    auto &v{kb.visuals[i]};
    bool down{v.active};

    ImVec4 fill{
        Helper::fromConfigColor(down ? theme.activeFill : theme.idleFill)};
    ImVec4 text{
        Helper::fromConfigColor(down ? theme.activeText : theme.idleText)};
    ImVec4 outline{Helper::fromConfigColor(down ? theme.keyOutline.activeColor
                                                : theme.keyOutline.idleColor)};

    animationActive |=
        animateKeyColor(v.colors.fill, fill, hideOverlay, colorT, hideT);
    animationActive |=
        animateKeyColor(v.colors.text, text, hideOverlay, colorT, hideT);
    animationActive |=
        animateKeyColor(v.colors.outline, outline, hideOverlay, colorT, hideT);
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
    animationActive |= Helper::animate(v.scale, targetScale, t);
  }
  return animationActive;
}

static std::pair<ImVec2, ImVec2>
getTopBottom(const ImVec2 &center, const ImVec2 &halfSize, float ovScale) {
  ImVec2 topLeft((center.x - halfSize.x) * ovScale,
                 (center.y - halfSize.y) * ovScale);
  ImVec2 bottomRight((center.x + halfSize.x) * ovScale,
                     (center.y + halfSize.y) * ovScale);
  return std::pair<ImVec2, ImVec2>{topLeft, bottomRight};
}

bool updateAnalogAnimations(const Layouts::Layout &layout, Keyboard::State &kb,
                            const Config::Theme &theme, float ovScale) {
  if (!theme.analog.enabled)
    return false;

  bool active{false};
  for (std::size_t i{0}; i < layout.keys.size(); ++i) {
    auto &v{kb.visuals[i]};
    const auto &key{layout.keys[i]};

    const float prev{v.analog.value};
    const float cur{
        std::clamp(Analog::readKey(layout.keys[i].code), 0.0f, 1.0f)};
    v.analog.value = cur;

    active |= (cur > 0.0f) || (cur != prev);
  }
  return active;
}
} // namespace Keyboard

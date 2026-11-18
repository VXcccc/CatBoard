#pragma once

#include "Constants.h"
#include "Fonts.h"
#include "Helpers.h"
#include "Hotkeys.h"
#include "Images.h"
#include "Layouts.h"
#include "Mouse.h"
#include "Sound.h"
#include <array>
#include <string>
#include <string_view>
#include <vector>
#include <winuser.h>

namespace Helper {
enum class FileCreateResult;

}

namespace Config {

using ColorRGBA = std::array<int, 4>;

struct Outline {
  bool enable{false};
  ColorRGBA idleColor{255, 255, 255, 255};
  ColorRGBA activeColor{0, 0, 0, 255};
  float thickness{3.0f};
};

struct Trail {
  bool enable{false};
  float duration{0.6f};
  float maxLength{150.0f};

  ColorRGBA color{255, 255, 255, 255};
  Mouse::TrailStyle style{};

  float minWidth{1.0f};
  float maxWidth{3.0f};
  int subdivisions{5};
};

struct MouseTheme {
  bool enabled{true};

  Mouse::MouseStyle style{};
  ColorRGBA outer{255, 255, 255, 255};
  ColorRGBA inner{255, 255, 255, 255};

  ColorRGBA fill{26, 26, 26, 200};

  bool lerpColor{false};

  float smallRadius{7.4f};
  float outerThickness{3.0f};
  float sensitivity{0.5f};
  float decayRate{8.0f};

  bool imagesDisplay{false};
  int imageOpacity{255};
  float imageScale{1.0f};

  float imageOffsetX{0.0f};
  float imageOffsetY{-50.0f};

  float fadeTime{0.6f};
  float rounding{6.0f};

  Trail trail{};

  bool highlightM1{true};
  ColorRGBA highlightColor{255, 255, 255, 255};
  float higlightScale{1.25f};

  ColorRGBA scrollCounter{0, 0, 0, 255};
};

enum class AnimType { Scale, Lift, MaxAnimTypes };

struct Theme {
  ColorRGBA idleFill{0, 0, 0, 255};
  ColorRGBA activeFill{255, 255, 255, 255};

  ColorRGBA idleText{255, 255, 255, 255};
  ColorRGBA activeText{0, 0, 0, 255};

  float keyRounding{0.0f};
  Outline keyOutline{};

  ColorRGBA obsBackground{0, 255, 0, 255};

  MouseTheme mouse{};

  bool keysImagesDisplay{false};
  int keysImageOpacity{255};

  bool animation{false};
  float pressScale{1.2f};
  int animationSpeed{100};
  bool animateColors{false};
  AnimType animType{AnimType::Scale};
};

struct SoundPackSelection {
  std::string name{Sound::defaultSoundPack};
  bool enable{false};
  bool layoutKeysOnlySound{false};
  float volume{1.0f};
};

struct Overlay {
  int x{0};
  int y{600};
  float scale{1.0f};

  bool operator==(const Overlay &) const = default;
};

struct Misc {
  bool vsync{true};
  bool obsMode{false};
  int fps{60};
  bool saveOnClose{true};
};

struct Settings {
  int version{Consts::CONFIG_VERSION};
  Theme theme{};
  std::string selectedLayout{Layouts::defaultLayoutName};
  SoundPackSelection soundPack{};
  Binds::BindsVector binds{Helper::arrToVec(Binds::defaultBinds)};
  Overlay overlay{};
  Fonts::Font font{};
  Misc misc{};
  std::string configName{};
  Images::Pack imagePack{};
};

// available config file names
extern std::vector<std::string> configs;
extern const Settings defaultConfig;

bool Load(std::string_view name, Settings &outConfig);
bool Save(std::string_view name, const Config::Settings &settings);
bool Delete(std::string_view name);
Helper::FileCreateResult Create(std::string_view name);
void refreshConfigs();

void LoadActiveConfig(Settings &outConfig);
bool Apply(const Settings &settings);

} // namespace Config

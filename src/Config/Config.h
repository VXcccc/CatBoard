#ifndef CONFIG_H
#define CONFIG_H

// maybe goona change location of this variable later

#include "Config/Constants.h"
#include "Input/Hotkeys.h"
#include "Input/Mouse.h"
#include "Media/Fonts.h"
#include "Media/Images.h"
#include "Media/Sound.h"
#include "UI/Layouts.h"
#include "imgui.h"
#include <array>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <winuser.h>
using json = nlohmann::json;

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
  bool enable{true};
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

  float radius{37.0f};
  float smallRadius{7.4f};
  float outerThickness{3.0f};
  float sensitivity{0.5f};
  float decayRate{8.0f};

  bool imagesDisplay{false};
  int imageOpacity{255};

  float imageOffsetX{0.0f};
  float imageOffsetY{-50.0f};

  float width{100.0f};
  float height{75.0f};

  float fadeTime{0.6f};
  float rounding{6.0f};

  Trail trail{};

  bool highlightM1{true};
  ColorRGBA highlightColor{255, 255, 255, 255};
  float higlightScale{1.25f};
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

struct OverlayPosition {
  int x{0};
  int y{600};
};

struct Misc {
  bool vsync{false};
  bool obsMode{false};
  int fps{60};
};

struct Settings {
  int version{Consts::CATBOARD_VERSION};
  Theme theme{};
  std::string selectedLayout{Layouts::defaultLayoutName};
  SoundPackSelection soundPack{};
  Binds::BindsArray binds{Binds::defaultBinds};
  OverlayPosition overlayPos{};
  Fonts::Font font{};
  Misc misc{};
  std::string configName{"Default"};
  Images::Pack imagePack{};
};

// current config
extern Settings activeConfig;
extern std::vector<std::string> configs;

bool loadConfig(std::string_view name);
bool saveConfig(std::string_view name, const Config::Settings &settings);
bool deleteConfig(std::string_view name);
Helper::FileCreateResult createConfig(std::string_view name);
void refreshConfigs();

void Load();
bool Apply(const Settings &settings);
} // namespace Config

#endif // !CONFIG_H

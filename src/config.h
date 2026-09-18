#ifndef CONFIG_H
#define CONFIG_H

// maybe goona change location of this variable later

#include "Constants.h"
#include "FontHandle.h"
#include "InputHandle.h"
#include "Paths.h"
#include "Sound.h"
#include "imgui.h"
#include "nlohmann/detail/macro_scope.hpp"
#include <array>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <winuser.h>
using json = nlohmann::json;

namespace Config {

enum class CreateConfigResult {
  Success,
  EmptyName,
  NameTooLong,
  AlreadyExists,
  DirectoryError,
  FileError,
  Max_ConfigResults
};

struct Outline {
  using ColorRGBA = std::array<int, 4>;

  bool enable{false};
  ColorRGBA color{255, 255, 255, 255};
  float thickness{3.0f};
};

struct Theme {
  using ColorRGBA = std::array<int, 4>;
  ColorRGBA idleFill{1, 1, 1, 255};
  ColorRGBA activeFill{255, 255, 255, 255};

  ColorRGBA idleText{255, 255, 255, 255};
  ColorRGBA activeText{1, 1, 1, 255};

  float rounding{0.0f};

  Outline outline{};

  ColorRGBA obsBackground{0, 255, 0, 255};
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
};

struct Settings {
  int version{Consts::CATBOARD_VERSION};
  Theme theme{};
  std::string selectedLayout{Input::defaultLayoutName};
  SoundPackSelection soundPack{};
  Input::BindsArray binds{Input::defaultBinds};
  OverlayPosition overlayPos{};
  Fonts::Font font{};
  Misc misc{};
};

// current config
extern Settings activeConfig;
extern std::vector<std::string> configs;
extern std::string activeConfigName;

bool loadConfig(std::string_view name);
bool saveConfig(std::string_view name, const Config::Settings &settings);
bool deleteConfig(std::string_view name);
CreateConfigResult createConfig(std::string_view name);
void refreshConfigs();

void Load();
bool Apply(const Settings &settings);
ImU32 ToImU32(const Config::Theme::ColorRGBA &c);
} // namespace Config

#endif // !CONFIG_H

#ifndef CONFIG_H
#define CONFIG_H

// maybe goona change location of this variable later

#include "Constants.h"
#include "Paths.h"
#include "imgui.h"
#include "nlohmann/detail/macro_scope.hpp"
#include <array>
#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;

namespace Config {

struct Theme {
  using ColorRGBA = std::array<int, 4>;
  ColorRGBA idleFill{1, 1, 1, 255};
  ColorRGBA activeFill{255, 255, 255, 255};

  ColorRGBA idleText{255, 255, 255, 255};
  ColorRGBA activeText{1, 1, 1, 255};

  float rounding{0.0f};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Theme, idleFill, activeFill, idleText,
                                   activeText, rounding)

struct Settings {
  int version{Consts::CATBOARD_VERSION};
  Theme theme{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings, version, theme)

// struct for user Configs
struct Profile {
  std::string name{};
  Settings settings{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Profile, name, settings)

// current config
extern Settings activeConfig;
extern std::vector<std::string> configs;

bool loadConfig(std::string_view name);
bool saveConfig(std::string_view name, const Config::Settings &settings);
bool deleteConfig(std::string_view name);
bool createConfig(std::string_view name);

void Load();
bool Save(const Settings &settings);
ImU32 ToImU32(const Config::Theme::ColorRGBA &c);
} // namespace Config

#endif // !CONFIG_H

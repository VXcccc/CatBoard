#ifndef CONFIG_H
#define CONFIG_H

// maybe goona change location of this variable later

#include <Constants.h>
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <array>
using json = nlohmann::json;

namespace Config {
// struct Color {
//   int r;
//   int g;
//   int b;
//   int o;
// };
// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Color, r, g, b, o)

struct Theme {
  typedef std::array<int, 4> ColorRGBA;
  ColorRGBA idleFill{1, 1, 1, 255};
  ColorRGBA activeFill{255, 255, 255, 255};

  ColorRGBA idleText{255, 255, 255, 255};
  ColorRGBA activeText{1, 1, 1, 255};

  float rounding{0.0f};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Theme, idleFill, activeFill, idleText, activeText, rounding)

struct Settings {
  int version{Consts::CATBOARD_VERSION};
  Theme theme{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings, version, theme)

extern Settings config;

void Load();
bool Save(const Settings &settings);
// ImU32 toImColor(const Color &c);
ImU32 ToImU32(const Config::Theme::ColorRGBA &c);
} // namespace Config

#endif // !CONFIG_H

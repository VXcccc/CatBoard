#ifndef CONFIG_H
#define CONFIG_H

// maybe goona change location of this variable later
#define CATBOARD_VERSION 1

#include "imgui.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace Config {
struct Color {
  int r;
  int g;
  int b;
  int o;
  ImU32 ToImU32() const { return IM_COL32(r, g, b, o); }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Color, r, g, b, o)

struct Theme {
  Color idleFillCol{1, 1, 1, 255};
  Color activeFillCol{255, 255, 255, 255};

  Color idleTextCol{255, 255, 255, 255};
  Color activeTextCol{1, 1, 1, 255};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Theme, activeFillCol, idleFillCol)

struct Settings {
  int version{CATBOARD_VERSION};
  Theme theme{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings, version, theme)

extern Settings config;

void Load();
bool Save(const Settings &settings);
ImU32 toImColor(const Color &c);

} // namespace Config

#endif // !CONFIG_H

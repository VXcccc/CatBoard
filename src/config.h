#ifndef CONFIG_H
#define CONFIG_H

#include "imgui.h"
#include "nlohmann/detail/macro_scope.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <fstream>
#include <vector>

namespace Config {
struct Color {
  int r;
  int g;
  int b;
  int o;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Color, r, g, b, o)

struct Theme {
  Color activeFillCol{255, 255, 255, 255};
  Color idleFillCol{1, 1, 1, 255};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Theme, activeFillCol, idleFillCol)

struct Settings {
  double version{1};
  Theme theme;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings, version, theme)

inline std::ifstream file{"config.json"};

inline Settings layout{json::parse(file).get<Settings>()};
} // namespace Config

#endif // !CONFIG_H

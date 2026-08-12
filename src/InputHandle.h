#include "nlohmann/detail/macro_scope.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <fstream>
// #include <windows.h>
#include <vector>
#ifndef INPUTHANDLE_H
#define INPUTHANDLE_H

namespace Input {
struct Key {
  int keyCode;
  std::string label;
  float x;
  float y;
  float w;
  float h;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Key, keyCode, label, x, y, w, h)

struct Layout {
    int windowWidth;
    int windowHeight;
    std::vector<Key> keys;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Layout, windowWidth, windowHeight, keys)

inline std::ifstream file{"layouts/layout7tNoMouse.json"};

inline Layout layout {json::parse(file).get<Layout>()};

// inline constexpr Input::Key keys[]  {
//   {'W', "W", 60, 10, 50, 50},
//   {'A', "A", 5, 65, 50, 50},
//   {'S', "S", 60, 65, 50, 50},
//   {'D', "D", 115, 65, 50, 50},
//   {VK_SPACE, "SPACE", 5, 120, 160, 50},
// };
} // namespace Input

#endif // !INPUTHANDLE_H

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

inline std::ifstream file{"Layouts/layout7tNoMouse.json"};

inline Layout layout {json::parse(file).get<Layout>()};

} // namespace Input

#endif // !INPUTHANDLE_H

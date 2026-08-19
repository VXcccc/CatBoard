#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <fstream>
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

extern std::vector<std::string> layouts;
extern Layout activeLayout;
extern std::string defaultLayoutName;

bool installDefaultLayouts();
void refreshLayouts();
bool loadLayout(std::string_view name);

} // namespace Input

#endif // !INPUTHANDLE_H

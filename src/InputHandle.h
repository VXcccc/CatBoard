#ifndef INPUTHANDLE_H
#define INPUTHANDLE_H

#include "Helpers.h"
#include "nlohmann/detail/macro_scope.hpp"
#include <nlohmann/json.hpp>
#include <windows.h>
#include <winuser.h>
using json = nlohmann::json;
#include <vector>

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

enum class BindType { dragMode, hideOverlay, openSettings, max_bindTypes };

struct Bind {
  std::string bindName;
  int vkCode;
  std::string keyName;
  BindType type;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Bind, bindName, vkCode, keyName, type);

using BindsArray =
    std::array<Bind, static_cast<std::size_t>(BindType::max_bindTypes)>;

extern BindsArray defaultBinds;

extern std::vector<std::string> layouts;
extern Layout activeLayout;
extern std::string defaultLayoutName;

bool installDefaultLayouts();
void refreshLayouts();
bool loadLayout(std::string_view name);

bool recordBind(Bind &outBind);

void handleBinds(const std::array<bool, 256> &prevKeyState,
                 const std::array<bool, 256> &currentKeysDown, HWND hWnd);

} // namespace Input

#endif // !INPUTHANDLE_H

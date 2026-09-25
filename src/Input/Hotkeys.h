#ifndef INPUTHANDLE_H
#define INPUTHANDLE_H

#include "nlohmann/detail/macro_scope.hpp"
#include <nlohmann/json.hpp>
#include <windows.h>
#include <winuser.h>
using json = nlohmann::json;

namespace Binds {
//binds
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

extern bool g_bindJustRecorded;

bool recordBind(Bind &outBind);

void handleBinds(const std::array<bool, 256> &prevKeyState,
                 const std::array<bool, 256> &currentKeysDown, HWND hWnd);

} // namespace Input

#endif // !INPUTHANDLE_H

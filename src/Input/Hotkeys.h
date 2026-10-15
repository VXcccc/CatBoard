#ifndef INPUTHANDLE_H
#define INPUTHANDLE_H

#include <string>
#include <vector>
#include <windows.h>
#include <winuser.h>

namespace Binds {
// binds
enum class BindType {
  dragMode,
  hideOverlay,
  openSettings,
  resizeMode,
  max_bindTypes
};

struct Bind {
  std::string name;
  int vkCode;
  std::string keyName;
  BindType type;
  bool operator==(const Bind &) const = default;
};

using BindsArray =
    std::array<Bind, static_cast<std::size_t>(BindType::max_bindTypes)>;

using BindsVector = std::vector<Bind>;

extern const BindsArray defaultBinds;

extern bool g_bindJustRecorded;

bool recordBind(Bind &outBind);

void handleBinds(const std::array<bool, 256> &prevKeyState,
                 const std::array<bool, 256> &currentKeysDown, HWND hWnd);

void syncBindsOrder(BindsVector& outBinds);
} // namespace Binds

#endif // !INPUTHANDLE_H

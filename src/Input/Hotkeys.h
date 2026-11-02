#ifndef INPUTHANDLE_H
#define INPUTHANDLE_H

#include <array>
#include <cstddef>
#include <string>
#include <vector>
#include <windows.h>
#include <winuser.h>

struct AppState;

namespace Binds {
// binds
enum class BindType {
  dragMode,
  toggleOverlay,
  toggleSettings,
  max_bindTypes
};

struct Bind {
  std::string name;
  int vkCode;
  std::string keyName;
	bool empty;
  BindType type;
  bool operator==(const Bind &) const = default;
};

using BindsArray =
    std::array<Bind, static_cast<std::size_t>(BindType::max_bindTypes)>;

using BindsVector = std::vector<Bind>;

extern const BindsArray defaultBinds;

bool recordBind(Bind &outBind, const BindsVector &binds, AppState &app);

void handleBinds(HWND hWnd, AppState &app);

void syncBindsOrder(BindsVector &outBinds);
} // namespace Binds

#endif // !INPUTHANDLE_H

#pragma once

#include "Constants.h"
#include <array>
#include <bitset>
#include <cstddef>
#include <string>
#include <vector>
#include <windows.h>
#include <winuser.h>

struct AppState;

namespace Keyboard {
using KeyBitSet = std::bitset<Consts::VK_COUNT>;
}

namespace Binds {
// binds

enum class BindType {
  dragMode,
  toggleOverlay,
  toggleSettings,
  autoHide,
  exit,
  MaxbindTypes
};

struct Bind {
  std::string name;
  int vkCode;
  std::string keyName;
  bool empty;
  BindType type;
  std::vector<int> modifierKeys{};
  bool operator==(const Bind &) const = default;
};

struct RecordingState {
  Bind *recordingBind{nullptr};
  Bind recordingPrev{};
	Keyboard::KeyBitSet recordingWasDown{};
};

using BindsArray =
    std::array<Bind, static_cast<std::size_t>(BindType::MaxbindTypes)>;

using BindsVector = std::vector<Bind>;

extern const BindsArray defaultBinds;

bool recordBind(Bind &outBind, const BindsVector &binds, AppState &app);

void handleBinds(HWND hWnd, AppState &app);

BindsVector syncBindsOrder(const BindsVector &prevBinds);
} // namespace Binds


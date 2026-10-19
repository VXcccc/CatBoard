#include "Input/Hotkeys.h"
#include "Config/Config.h"
#include "Input/Keyboard.h"
#include "System/Helpers.h"
#include "System/Tray.h"
#include "UI/MainWindow.h"
#include "UI/SettingsWindow.h"
#include <array>
#include <nlohmann/json.hpp>
#include <windows.h>
using json = nlohmann::json;

namespace Binds {

static bool s_bindJustRecorded{false};

const BindsArray defaultBinds{
    Bind{"Drag mode", VK_F4, "F4", BindType::dragMode},
    Bind{"Hide overlay", VK_F6, "F6", BindType::hideOverlay},
    Bind{"Open settings", VK_F9, "F9", BindType::openSettings},
    Bind{"Resize mode", VK_F10, "F10", BindType::resizeMode}};

static_assert(defaultBinds.size() ==
                  static_cast<std::size_t>(BindType::max_bindTypes),
              "default binds array out of sync with bind type enum");

void handleBindType(const BindType type, HWND hWnd) {
  switch (type) {
  case BindType::dragMode:
    MainWindow::g_dragMode = !MainWindow::g_dragMode;
    MainWindow::SetDragMode(hWnd, MainWindow::g_dragMode);
    break;
  case BindType::hideOverlay:
    if (!MainWindow::IsVisible()) {
      MainWindow::Show();
    } else {
      MainWindow::Hide();
    }
    break;
  case BindType::openSettings:
    if (!SettingsWindow::IsVisible()) {
      SettingsWindow::Show();
    } else {
      SettingsWindow::Hide();
    }
    break;
  case BindType::resizeMode:
    MainWindow::g_resizeMode = !MainWindow::g_resizeMode;
    MainWindow::SetResizeMode(hWnd, MainWindow::g_resizeMode);

  default:
    break;
  }
}

void handleBinds(const Keyboard::State &kb, HWND hWnd) {
  if (s_bindJustRecorded) {
    s_bindJustRecorded = false;
    return;
  }
  for (const auto &bind : Config::activeConfig.binds) {
    if (kb.inRange(bind.vkCode) && kb.pressed(bind.vkCode)) {
      handleBindType(bind.type, hWnd);
    }
  }
}

bool isBindFree(int vk) {
  for (const auto &bind : Config::activeConfig.binds) {
    if (vk == bind.vkCode) {
      return false;
    }
  }
  return true;
}

bool recordBind(Bind &outBind) {
  Bind prevBindState{outBind};

  std::array<bool, 256> wasDown{};
  for (int vk = 0; vk < 256; ++vk) {
    wasDown[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
  }

  while (true) {
    bool escapeDown{(GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0};

    if (!wasDown[VK_ESCAPE] && escapeDown) {
      outBind = prevBindState;
      return true;
    }

    for (int vk{0}; vk < 256; ++vk) {
      bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;

      if (wasDown[vk]) {
        if (!down) {
          wasDown[vk] = false;
        }
        continue;
      }

      if (down) {
        if (Helper::VkToName(vk) == "Unknown" || !isBindFree(vk)) {
          outBind = prevBindState;
          s_bindJustRecorded = true;

          return false;
        }

        outBind.vkCode = vk;
        outBind.keyName = Helper::VkToName(vk);

        s_bindJustRecorded = true;

        return true;
      }
    }
  }
  outBind = prevBindState;
  return false;
}

// add for compatibility with old configs if new bind added
void syncBindsOrder(BindsVector &outBinds) {
  const auto previousBinds{outBinds};

  BindsVector ordered;
  ordered.reserve(Binds::defaultBinds.size());

  for (const auto &defaultBind : Binds::defaultBinds) {
    const auto it = std::find_if(previousBinds.begin(), previousBinds.end(),
                                 [&defaultBind](const Bind &bind) {
                                   return (bind.type == defaultBind.type &&
                                           bind.name == defaultBind.name);
                                 });

    if (it != previousBinds.end()) {
      ordered.push_back(*it);
    } else {
      ordered.push_back(defaultBind);
    }
  }

  outBinds = std::move(ordered);
}

} // namespace Binds

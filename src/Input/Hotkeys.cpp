#include "Input/Hotkeys.h"
#include "System/Helpers.h"
#include "UI/MainWindow.h"
#include "UI/SettingsWindow.h"
#include "System/Tray.h"
#include <array>
#include <nlohmann/json.hpp>
#include <windows.h>
using json = nlohmann::json;

namespace Binds {

bool g_bindJustRecorded{false};

BindsArray defaultBinds{
    Bind{"Drag mode", VK_F4, "F4", BindType::dragMode},
    Bind{"Hide overlay", VK_F6, "F6", BindType::hideOverlay},
    Bind{"Open settings", VK_F9, "F9", BindType::openSettings}};


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
  default:
    break;
  }
}

void handleBinds(const std::array<bool, 256> &prevKeyState,
                 const std::array<bool, 256> &currentKeysDown, HWND hWnd) {
  for (const auto &bind : Config::activeConfig.binds) {
    bool down{currentKeysDown[bind.vkCode]};
    if (down && !prevKeyState[bind.vkCode]) {
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
          g_bindJustRecorded = true;

          return false;
        }

        outBind.vkCode = vk;
        outBind.keyName = Helper::VkToName(vk);

        g_bindJustRecorded = true;

        return true;
      }
    }
  }
  outBind = prevBindState;
  return false;
}

} // namespace Input

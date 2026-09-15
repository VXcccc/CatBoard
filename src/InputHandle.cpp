#include "InputHandle.h"
#include "Config.h"
#include "Helpers.h"
#include "MainWindow.h"
#include "Paths.h"
#include "SettingsWindow.h"
#include "SystemTray.h"
#include <array>
#include <nlohmann/json.hpp>
#include <windows.h>
using json = nlohmann::json;
#include <fstream>
#include <string_view>
#include <vector>

namespace Input {
std::vector<std::string> layouts{};
Layout activeLayout{};
std::string defaultLayoutName{"layout7t"};

bool g_bindJustRecorded{false};

BindsArray defaultBinds{
    Bind{"Drag mode", VK_F4, "F4", BindType::dragMode},
    Bind{"Hide overlay", VK_F6, "F6", BindType::hideOverlay},
    Bind{"Open settings", VK_F9, "F9", BindType::openSettings}};

void refreshLayouts() {
  Helper::refresh(layouts, Paths::layoutDirectory(), ".json");
}

bool installDefaultLayouts() {
  const auto source{Paths::getExecutableDir() /
                    "Layouts"}; // TODO fix this function(cuz idk how its gonna
                                // work without layouts dir in executable dir)
  const auto destination{Paths::layoutDirectory()};

  if (std::filesystem::exists(destination)) {
    return true;
  }

  std::error_code ec;

  std::filesystem::copy(source, destination,
                        std::filesystem::copy_options::recursive, ec);

  return !ec;
}

bool loadLayoutFile(std::string_view name) {
  try {
    std::ifstream file{Paths::getLayoutPath(name)};

    if (!file)
      return false;

    activeLayout = json::parse(file).get<Layout>();
    return true;
  } catch (const std::exception &) {
    return false;
  }
}

bool loadLayout(std::string_view name) {
  if (loadLayoutFile(name))
    return true;

  if (name != defaultLayoutName)
    return loadLayoutFile(defaultLayoutName);

  return false;
}

void handleBindType(const BindType type, HWND hWnd) {
  switch (type) {
  case BindType::dragMode:
    Tray::g_dragMode = !Tray::g_dragMode;
    Tray::SetDragMode(hWnd, Tray::g_dragMode);
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

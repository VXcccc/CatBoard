#include "Hotkeys.h"
#include "AppState.h"
#include "Config.h"
#include "Constants.h"
#include "Helpers.h"
#include "Keyboard.h"
#include "MainWindow.h"
#include "SettingsWindow.h"
#include "Tray.h"
#include <array>
#include <windows.h>
namespace Binds {

const BindsArray defaultBinds{
    Bind{"Drag mode", VK_F4, "F4", false, BindType::dragMode},
    Bind{"Toggle overlay", VK_F6, "F6", false, BindType::toggleOverlay},
    Bind{"Toggle settings", VK_F9, "F9", false, BindType::toggleSettings}};

static_assert(defaultBinds.size() ==
                  static_cast<std::size_t>(BindType::max_bindTypes),
              "default binds array out of sync with bind type enum");

void handleBindType(const BindType type, HWND hWnd, AppState &app) {
  switch (type) {
  case BindType::dragMode:
    app.dragMode = !app.dragMode;
    MainWindow::SetDragMode(hWnd, app.dragMode);
    break;
  case BindType::toggleOverlay:
    MainWindow::toogleOverlay();
    break;
  case BindType::toggleSettings:
    SettingsWindow::toggleWindow();
    break;
    // case BindType::resizeMode:
    //   app.resizeMode = !app.resizeMode;
    //   MainWindow::SetResizeMode(hWnd, app.resizeMode);

  default:
    break;
  }
}

void handleBinds(HWND hWnd, AppState &app) {
  if (app.bindJustRecorded) {
    app.bindJustRecorded = false;
    return;
  }

  for (const auto &bind : app.config.binds) {
    if (app.keyboard.inRange(bind.vkCode) &&
        app.keyboard.pressed(bind.vkCode) && !bind.empty) {
      handleBindType(bind.type, hWnd, app);
    }
  }
}

bool isBindFree(int vk, const BindsVector &binds) {
  for (const auto &bind : binds) {
    if (vk == bind.vkCode && !bind.empty) {
      return false;
    }
  }
  return true;
}

static void makeEmpty(Bind &outBind) {
  outBind.empty = true;
  outBind.keyName = Consts::EMPTY_HOTKEY_NAME;
}

bool recordBind(Bind &outBind, const BindsVector &binds, AppState &app) {
  Bind prevBindState{outBind};

  std::array<bool, 256> wasDown{};
  for (int vk = 0; vk < 256; ++vk) {
    wasDown[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
  }

  while (true) {
    bool escapeDown{(GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0};

    if (!wasDown[VK_ESCAPE] && escapeDown) {
      makeEmpty(outBind);
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
        if (Helper::VkToName(vk) == "Unknown") {
          makeEmpty(outBind);
          app.bindJustRecorded = true;
          return false;
        } else if (!(isBindFree(vk, binds))) {
          outBind = prevBindState;
          app.bindJustRecorded = true;
          return false;
        }

        outBind.empty = false;
        outBind.vkCode = vk;
        outBind.keyName = Helper::VkToName(vk);

        app.bindJustRecorded = true;

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

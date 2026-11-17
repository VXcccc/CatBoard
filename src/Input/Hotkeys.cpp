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
#include <vector>
#include <windows.h>

namespace Binds {

// If you want to add custom hotkey add new Type in BindType also add new Bind
// to this Array and finally add functionality that you want your bind execute
// in handleBindType() switch statement
const BindsArray defaultBinds{
    Bind{"Drag mode",
         VK_F4,
         Consts::EMPTY_HOTKEY_NAME,
         true,
         BindType::dragMode,
         {}},
    Bind{"Toggle overlay", VK_F6, "F6", false, BindType::toggleOverlay, {}},
    Bind{"Toggle settings",
         VK_F9,
         Consts::EMPTY_HOTKEY_NAME,
         true,
         BindType::toggleSettings,
         {}},
    Bind{"Exit", VK_F12, Consts::EMPTY_HOTKEY_NAME, true, BindType::exit, {}}};

static_assert(defaultBinds.size() ==
                  static_cast<std::size_t>(BindType::MaxbindTypes),
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
  case BindType::exit:
    PostMessageW(hWnd, WM_CLOSE, 0, 0);
    break;
    // case BindType::resizeMode:
    //   app.resizeMode = !app.resizeMode;
    //   MainWindow::SetResizeMode(hWnd, app.resizeMode);

  default:
    break;
  }
}

static bool isModifersPressed(const std::vector<int> &modifierKeys,
                              const AppState &app) {
  if (modifierKeys.empty())
    return true;
  for (const int mKey : modifierKeys) {
    if (!app.keyboard.down[mKey]) {
      return false;
    }
  }
  return true;
}

void handleBinds(HWND hWnd, AppState &app) {
  if (app.bindJustRecorded) {
    app.bindJustRecorded = false;
    return;
  }

  for (const auto &bind : app.config.binds) {
    if (app.keyboard.inRange(bind.vkCode) &&
        app.keyboard.pressed(bind.vkCode) && !bind.empty &&
        isModifersPressed(bind.modifierKeys, app)) {
      handleBindType(bind.type, hWnd, app);
    }
  }
}

bool isBindFree(int vk, const BindsVector &binds,
                const std::vector<int> &modifierKeys) {
  for (const auto &existingBind : binds) {
    bool vkCodesEqual{vk == existingBind.vkCode};
    if (vkCodesEqual && !existingBind.empty &&
        existingBind.modifierKeys.empty()) {
      return false;
    } else if (!existingBind.modifierKeys.empty()) {
      if (modifierKeys == existingBind.modifierKeys && vkCodesEqual &&
          !existingBind.empty) {
        return false;
      }
    }
  }
  return true;
}

static void makeEmpty(Bind &outBind) {
  outBind.empty = true;
  outBind.keyName = Consts::EMPTY_HOTKEY_NAME;
}

static bool isKeyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

static bool isModifierVk(int vk) {
  return vk == VK_CONTROL || vk == VK_SHIFT || vk == VK_MENU ||
         (vk >= VK_LSHIFT && vk <= VK_RMENU); // 0xA0..0xA5
}

// maybe remove while loop from here add an recordingstate struct
bool recordBind(Bind &outBind, const BindsVector &binds, AppState &app) {
  // if you want to add more modifierKeys add vk codes to this array and change
  // isModifierVk function
  constexpr std::array modifierKeys{VK_CONTROL, VK_SHIFT, VK_MENU};

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
      bool down = isKeyDown(vk);

      if (wasDown[vk]) {
        if (!down) {
          wasDown[vk] = false;
        }
        continue;
      }

      if (isModifierVk(vk))
        continue;

      if (down) {
        if (Helper::VkToName(vk) == "Unknown") {
          makeEmpty(outBind);
          app.bindJustRecorded = true;
          return false;
        }

        // record all pressed modifierKeys
        std::vector<int> recordedMKeys{};
        for (const int mKey : modifierKeys) {
          if (isKeyDown(mKey))
            recordedMKeys.push_back(mKey);
        }

        if (!(isBindFree(vk, binds, recordedMKeys))) {
          outBind = prevBindState;
          app.bindJustRecorded = true;
          return false;
        }

        outBind.modifierKeys.clear();
        outBind.modifierKeys = std::move(recordedMKeys);

        outBind.empty = false;
        outBind.vkCode = vk;
        if (outBind.modifierKeys.empty()) {
          outBind.keyName = Helper::VkToName(vk);
        } else {
          std::string keyNameResult{};
          for (const int mKey : outBind.modifierKeys) {
            keyNameResult += Helper::VkToName(mKey);
            keyNameResult += " + ";
          }
          keyNameResult += Helper::VkToName(vk);

          outBind.keyName = keyNameResult;
        }

        app.bindJustRecorded = true;

        return true;
      }
    }
  }
  outBind = prevBindState;
  return false;
}

BindsVector syncBindsOrder(const BindsVector &prevBinds) {
  BindsVector ordered;
  ordered.reserve(Binds::defaultBinds.size());

  for (const auto &defaultBind : Binds::defaultBinds) {
    const auto it = std::find_if(prevBinds.begin(), prevBinds.end(),
                                 [&defaultBind](const Bind &bind) {
                                   return (bind.type == defaultBind.type &&
                                           bind.name == defaultBind.name);
                                 });

    if (it != prevBinds.end()) {
      ordered.push_back(*it);
    } else {
      ordered.push_back(defaultBind);
    }
  }

  return ordered;
}

} // namespace Binds

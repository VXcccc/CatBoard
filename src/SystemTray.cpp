#include "SystemTray.h"
#include "Constants.h"
#include <shellapi.h>
#include <strsafe.h>
#include <windows.h>
#include <winuser.h>

#include <iostream>

namespace Tray {
NOTIFYICONDATA g_nid{};
static bool g_pinned{true};

NOTIFYICONDATA init(HWND hwnd) {
  NOTIFYICONDATA nid{};
  nid.cbSize = sizeof(nid);
  nid.hWnd = hwnd;
  nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

  nid.uCallbackMessage = Consts::WM_TRAYICON;

  nid.hIcon = static_cast<HICON>(LoadImage(NULL, "Images/icon.ico", IMAGE_ICON,
                                           0, 0, LR_LOADFROMFILE | LR_SHARED));

  StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), "CatBoard");

  Shell_NotifyIcon(NIM_ADD, &nid);

  return nid;
}

VOID APIENTRY DisplayContextMenu(HWND hWnd) {
  POINT pt{};
  GetCursorPos(&pt);

  HMENU hmenu{CreatePopupMenu()};
  HMENU hSettingsMenu {CreatePopupMenu()};

  MENUITEMINFO s_pin{};
  s_pin.cbSize = sizeof(s_pin);
  s_pin.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
  s_pin.wID = Consts::ID_PIN;
  s_pin.fState = g_pinned ? MFS_CHECKED : MFS_UNCHECKED;
  s_pin.dwTypeData = const_cast<LPSTR>("Pin window");
  InsertMenuItem(hSettingsMenu, 0, true, &s_pin);

  MENUITEMINFO settings{};
  settings.cbSize = sizeof(settings);
  settings.fMask =  MIIM_STRING | MIIM_SUBMENU;
  settings.hSubMenu = hSettingsMenu;
  settings.dwTypeData = const_cast<LPSTR>("Settings");
  InsertMenuItem(hmenu, 0, true, &settings);

  MENUITEMINFO exit{};
  exit.cbSize = sizeof(exit);
  exit.fMask = MIIM_ID | MIIM_STRING;
  exit.wID = Consts::ID_EXIT;
  exit.dwTypeData = const_cast<LPSTR>("Exit");
  InsertMenuItem(hmenu, 1, true, &exit);

  SetForegroundWindow(hWnd);

  TrackPopupMenuEx(hmenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, hWnd,
                   NULL);

  PostMessage(hWnd, WM_NULL, 0, 0);
  // Destroy the menu.

  DestroyMenu(hmenu);
}

// On_MYWM_NOTIFYICON - processes callback messages for taskbar icons.
// wParam - first message parameter of the callback message.
// lParam - second message parameter of the callback message.
void On_TrayWM_NOTIFYICON(WPARAM wParam, LPARAM lParam, HWND hWnd) {
  // UINT uID;
  UINT uMouseMsg;

  // uID = (UINT)wParam;
  uMouseMsg = (UINT)lParam;

  switch (uMouseMsg) {
  case WM_LBUTTONUP:
    DisplayContextMenu(hWnd);
    return;
  case WM_RBUTTONUP:
    DisplayContextMenu(hWnd);
    return;
  }

  return;
}

void On_WM_COMMAND(WPARAM wParam, LPARAM lParam, HWND hWnd) {
  UINT uId { static_cast<UINT>(LOWORD(wParam))};
  std::cout << uId;
  switch (uId) {
  case Consts::ID_PIN:
    std::cout << "Unpinned";
    return;
  case Consts::ID_EXIT:
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
    PostQuitMessage(0);
    return;
  }
}
} // namespace Tray

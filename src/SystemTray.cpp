#include "SystemTray.h"
#include "Constants.h"
#include <basetsd.h>
#include <shellapi.h>
#include <strsafe.h>
#include <windows.h>
#include <winuser.h>

#include <iostream>

namespace Tray {
NOTIFYICONDATA g_nid{}; 
bool g_dragMode{false};

NOTIFYICONDATA init(HWND hwnd) {
  NOTIFYICONDATA nid{};
  nid.cbSize = sizeof(nid);
  nid.hWnd = hwnd;
  nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

  nid.uCallbackMessage = Consts::WM_TRAYICON;

  nid.hIcon = static_cast<HICON>(LoadImage(NULL, L"Images/icon.ico", IMAGE_ICON,
                                           0, 0, LR_LOADFROMFILE | LR_SHARED));

  StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), L"CatBoard");

  Shell_NotifyIcon(NIM_ADD, &nid);

  return nid;
}

VOID APIENTRY DisplayContextMenu(HWND hWnd) {
  POINT pt{};
  GetCursorPos(&pt);

  HMENU hMenu{CreatePopupMenu()};
  HMENU hSettingsMenu {CreatePopupMenu()};

  MENUITEMINFO s_drag{};
  s_drag.cbSize = sizeof(s_drag);
  s_drag.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
  s_drag.wID = Consts::ID_DRAGMODE;
  s_drag.fState = g_dragMode ? MFS_CHECKED : MFS_UNCHECKED;
  s_drag.dwTypeData = const_cast<LPWSTR>(L"Drag mode");
  InsertMenuItem(hMenu, 0, true, &s_drag);

  MENUITEMINFO settings{};
  settings.cbSize = sizeof(settings);
  settings.fMask =  MIIM_STRING | MIIM_SUBMENU;
  settings.hSubMenu = hSettingsMenu;
  settings.dwTypeData = const_cast<LPWSTR>(L"Settings");
  InsertMenuItem(hMenu, 1, true, &settings);

  MENUITEMINFO exit{};
  exit.cbSize = sizeof(exit);
  exit.fMask = MIIM_ID | MIIM_STRING;
  exit.wID = Consts::ID_EXIT;
  exit.dwTypeData = const_cast<LPWSTR>(L"Exit");
  InsertMenuItem(hMenu, 2, true, &exit);

  SetForegroundWindow(hWnd);

  TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, hWnd,
                   NULL);

  PostMessage(hWnd, WM_NULL, 0, 0);
  // Destroy the menu.

  DestroyMenu(hMenu);
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
void SetDragMode(HWND hWnd, bool enabled){
  LONG_PTR exStyle {GetWindowLongPtr(hWnd, GWL_EXSTYLE)};
  if(enabled){
    exStyle &= ~WS_EX_TRANSPARENT;
  }else{
    exStyle |= WS_EX_TRANSPARENT;
  }
  SetWindowLongPtr(hWnd, GWL_EXSTYLE, exStyle);
  SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void On_WM_COMMAND(WPARAM wParam, LPARAM lParam, HWND hWnd) {
  UINT uId { static_cast<UINT>(LOWORD(wParam))};
  std::cout << uId;
  switch (uId) {
  case Consts::ID_DRAGMODE:
    g_dragMode = !g_dragMode;
    SetDragMode(hWnd, g_dragMode);
    return;
  case Consts::ID_EXIT:
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
    PostQuitMessage(0);
    return;
  }
}
} // namespace Tray

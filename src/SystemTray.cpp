#include "SystemTray.h"
#include "SettingsWindow.h"
#include "Constants.h"
#include <basetsd.h>
#include <ios>
#include <shellapi.h>
#include <strsafe.h>
#include <windows.h>
#include <winuser.h>

#include <iomanip>
#include <iostream>

namespace Tray {
NOTIFYICONDATA g_nid{};
bool g_dragMode{false};
bool g_showSettings{false};
static int g_interactiveRequests{0};

class TrayMenu {
public:
  TrayMenu() : m_hMenu{CreatePopupMenu()} {}
  ~TrayMenu() { DestroyMenu(m_hMenu); }

  void addItem(UINT id, const wchar_t *label, bool hasState = false,
               bool checked = false) {
    MENUITEMINFO mii{};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_ID | MIIM_STRING | (hasState ? MIIM_STATE : 0);
    mii.wID = id;
    mii.fState = checked ? MFS_CHECKED : MFS_UNCHECKED;
    mii.dwTypeData = const_cast<LPWSTR>(label);
    InsertMenuItem(m_hMenu, GetMenuItemCount(m_hMenu), true, &mii);
  }

  void showAt(HWND hWnd, POINT pt) {
    SetForegroundWindow(hWnd);
    TrackPopupMenuEx(m_hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, hWnd,
                     NULL);
    PostMessage(hWnd, WM_NULL, 0, 0);
  }

private:
  HMENU m_hMenu;
};

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

  TrayMenu menu;
  menu.addItem(Consts::ID_DRAGMODE, L"Drag mode", true, g_dragMode);
  menu.addItem(Consts::ID_SETTINGS, L"Settings");
  menu.addItem(Consts::ID_EXIT, L"Exit");
  menu.showAt(hWnd, pt);
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
void RequestInteractive(HWND hwnd) {
  g_interactiveRequests++;
  if (g_interactiveRequests == 1) {
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    exStyle &= ~WS_EX_TRANSPARENT;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
  }
}

void ReleaseInteractive(HWND hwnd) {
  g_interactiveRequests--;
  if (g_interactiveRequests == 0) {
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_TRANSPARENT;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
  }
}

void On_WM_COMMAND(WPARAM wParam, LPARAM lParam, HWND hWnd) {
  UINT uId{static_cast<UINT>(LOWORD(wParam))};

  switch (uId) {
  case Consts::ID_SETTINGS:
    if (!g_showSettings) {
      g_showSettings = true;
      RequestInteractive(hWnd);
    }
    return;
  case Consts::ID_DRAGMODE:
    g_dragMode = !g_dragMode;
    if (g_dragMode) {
      RequestInteractive(hWnd);
    } else {
      ReleaseInteractive(hWnd);
    }
    return;
  case Consts::ID_EXIT:
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
    PostQuitMessage(0);
    return;
  }
}
} // namespace Tray

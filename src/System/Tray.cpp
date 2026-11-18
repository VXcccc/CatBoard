#include "Tray.h"
#include "AppState.h"
#include "Constants.h"
#include "MainWindow.h"
#include "Resource/ResourceConstants.h"
#include "SettingsWindow.h"
#include <basetsd.h>
#include <minwindef.h>
#include <shellapi.h>
#include <strsafe.h>
#include <windows.h>
#include <winuser.h>

namespace Tray {
NOTIFYICONDATA g_nid{};
// static int g_interactiveRequests{0};

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

static HICON loadTrayIcon(HINSTANCE hInstance) {
  return static_cast<HICON>(LoadImage(GetModuleHandle(nullptr),
                                      MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0,
                                      0, LR_DEFAULTSIZE | LR_SHARED));
}

NOTIFYICONDATA init(HWND hwnd, HINSTANCE hInstance) {
  NOTIFYICONDATA nid = {};
  nid.cbSize = sizeof(nid);
  nid.hWnd = hwnd;
  nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  nid.uCallbackMessage = Consts::WM_TRAYICON;
  nid.hIcon = loadTrayIcon(hInstance);

  StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), L"CatBoard");

  Shell_NotifyIcon(NIM_ADD, &nid);

  return nid;
}

// unused function right now
void updateIcon(HINSTANCE hInstance) {
  HICON icon = loadTrayIcon(hInstance);
  if (!icon)
    return;

  g_nid.hIcon = icon;
  g_nid.uFlags = NIF_ICON;
  Shell_NotifyIcon(NIM_MODIFY, &g_nid);
  g_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
}

void remove() { Shell_NotifyIcon(NIM_DELETE, &g_nid); }

VOID APIENTRY DisplayContextMenu(HWND hWnd, const AppState &app) {
  POINT pt{};
  GetCursorPos(&pt);

  TrayMenu menu;
  menu.addItem(Consts::ID_DRAGMODE, L"Drag mode", true, app.dragMode);
  menu.addItem(Consts::ID_SETTINGS, L"Settings");
  menu.addItem(Consts::ID_EXIT, L"Exit");
  menu.showAt(hWnd, pt);
}

// On_MYWM_NOTIFYICON - processes callback messages for taskbar icons.
// wParam - first message parameter of the callback message.
// lParam - second message parameter of the callback message.
void On_TrayWM_NOTIFYICON(WPARAM wParam, LPARAM lParam, HWND hWnd,
                          AppState &app) {
  // UINT uID;
  UINT uMouseMsg;

  // uID = (UINT)wParam;
  uMouseMsg = (UINT)lParam;

  switch (uMouseMsg) {
  case WM_LBUTTONUP:
    MainWindow::toogleOverlay();
    return;
  case WM_RBUTTONUP:
    DisplayContextMenu(hWnd, app);
    return;
  }

  return;
}

void On_WM_COMMAND(WPARAM wParam, LPARAM lParam, HWND hWnd, AppState &app) {
  UINT uId{static_cast<UINT>(LOWORD(wParam))};

  switch (uId) {
  case Consts::ID_SETTINGS:
    SettingsWindow::Show();
    return;
  case Consts::ID_DRAGMODE:
    app.dragMode = !app.dragMode;
    MainWindow::SetDragMode(hWnd, app.dragMode);
    return;
  case Consts::ID_EXIT:
    PostMessageW(hWnd, WM_CLOSE, 0, 0);
    return;
  }
}
} // namespace Tray

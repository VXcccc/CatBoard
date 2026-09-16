#include "SystemTray.h"
#include "Constants.h"
#include "Helpers.h"
#include "InputHandle.h"
#include "MainWindow.h"
#include "SettingsWindow.h"
#include <basetsd.h>
#include <minwindef.h>
#include <shellapi.h>
#include <strsafe.h>
#include <windows.h>
#include <winuser.h>

namespace Tray {
NOTIFYICONDATA g_nid{};
bool g_dragMode{false};
bool g_showSettings{false};
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

NOTIFYICONDATA init(HWND hwnd, HINSTANCE hInstance) {
  NOTIFYICONDATA nid{};
  nid.cbSize = sizeof(nid);
  nid.hWnd = hwnd;
  nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

  nid.uCallbackMessage = Consts::WM_TRAYICON;
  nid.hIcon = static_cast<HICON>(
      LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0,
                LR_DEFAULTSIZE | LR_SHARED));
  // nid.hIcon = static_cast<HICON>(LoadImage(NULL, L"Images/icon.ico",
  // IMAGE_ICON,
  //                                          0, 0, LR_LOADFROMFILE |
  //                                          LR_SHARED));

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

// to do move this functions to mainWindow.h
void SetDragMode(HWND hWnd, bool enabled) {
  LONG_PTR exStyle{GetWindowLongPtr(hWnd, GWL_EXSTYLE)};
  if (enabled) {
    exStyle &= ~WS_EX_TRANSPARENT;
  } else {
    exStyle |= WS_EX_TRANSPARENT;
  }
  SetWindowLongPtr(hWnd, GWL_EXSTYLE, exStyle);
  SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void SetObsMode(HWND hWnd, bool enabled) {
  int windowWidth{
      (int)(Input::activeLayout.windowWidth * MainWindow::getG_MainScale())};

  int windowHeight{
      (int)(Input::activeLayout.windowHeight * MainWindow::getG_MainScale())};

  // adding 30 pixles cuz of sysmenu
  int obsWindowHeight{
      ((int)(Input::activeLayout.windowHeight * MainWindow::getG_MainScale())) +
      (30)};

  LONG_PTR exStyle{GetWindowLongPtr(hWnd, GWL_EXSTYLE)};
  LONG_PTR style{GetWindowLongPtr(hWnd, GWL_STYLE)};

  if (enabled) {
    exStyle &= ~(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST |
                 WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);

    style = (style & ~WS_POPUP) |
            (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME);

    MainWindow::setClearColor(
        ImVec4(0.0f, Helper::fromColorRGB(255), 0.0f, 1.0f));
  } else {
    exStyle |= (WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST |
                WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);

    style =
        (style & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME)) |
        WS_POPUP;
  }

  SetWindowLongPtr(hWnd, GWL_EXSTYLE, exStyle);
  SetWindowLongPtr(hWnd, GWL_STYLE, style);

  if (!enabled) {
    MainWindow::setClearColor(Consts::defaultClearColor);
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
  }

  SetWindowPos(hWnd, enabled ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, windowWidth,
               enabled ? obsWindowHeight : windowHeight,
               SWP_NOMOVE | SWP_FRAMECHANGED);
}

void On_WM_COMMAND(WPARAM wParam, LPARAM lParam, HWND hWnd) {
  UINT uId{static_cast<UINT>(LOWORD(wParam))};

  switch (uId) {
  case Consts::ID_SETTINGS:
    SettingsWindow::Show();
    return;
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

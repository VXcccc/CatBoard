// MainWindow namespace also used in Main.cpp (check main for more information)

#include "MainWindow.h"
#include "Config.h"
#include "Helpers.h"
#include "Layouts.h"
#include "SettingsWindow.h"
#include <windows.h>
#include <winuser.h>

namespace MainWindow {
bool g_dragMode{false};
bool g_resizeMode{false};

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

void SetResizeMode(HWND hWnd, bool enabled) {
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

// fix obs mode
void SetObsMode(HWND hWnd, bool enabled) {
  int windowWidth{
      (int)(Layouts::activeLayout.windowWidth * MainWindow::getG_MainScale())};

  int windowHeight{
      (int)(Layouts::activeLayout.windowHeight * MainWindow::getG_MainScale())};

  // adding 40 pixles cuz of sysmenu
  int obsWindowHeight{((int)(Layouts::activeLayout.windowHeight *
                             MainWindow::getG_MainScale())) +
                      (40)};

  LONG_PTR exStyle{GetWindowLongPtr(hWnd, GWL_EXSTYLE)};
  LONG_PTR style{GetWindowLongPtr(hWnd, GWL_STYLE)};

  if (enabled) {
    exStyle &= ~(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST |
                 WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);

    style = (style & ~WS_POPUP) |
            (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME);

    MainWindow::setClearColor(
        Helper::fromConfigColor(Config::activeConfig.theme.obsBackground));
  } else {
    exStyle |= (WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED |
                WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);

    style =
        (style & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME)) |
        WS_POPUP;
  }

  SetWindowLongPtr(hWnd, GWL_EXSTYLE, exStyle);
  SetWindowLongPtr(hWnd, GWL_STYLE, style);

  if (!enabled) {
    MainWindow::setClearColor(Consts::defaultClearColor);
  }

  SetWindowPos(hWnd, enabled ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, windowWidth,
               enabled ? obsWindowHeight : windowHeight,
               SWP_NOMOVE | SWP_FRAMECHANGED);
}

bool overlayChanged(Config::Overlay &outPrevOv, const Config::Overlay &curOv) {
  const bool changed{outPrevOv != curOv};

  if (changed) {
    MainWindow::ApplyOverlaySize();
    MainWindow::ApplyOverlayPosition();
    // ik the title bar is flashing when you changing the size or pos in
    // settings, but idk how to deal with unfocus other way
    if (SettingsWindow::IsVisible()) {
      SettingsWindow::ForceForeground(SettingsWindow::GetHWND());
    }
  }

  outPrevOv = curOv;
  return changed;
}

}; // namespace MainWindow

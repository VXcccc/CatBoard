// MainWindow namespace also used in Main.cpp (check main for more information)

#include "UI/MainWindow.h"
#include "Config/Config.h"
#include "System/Helpers.h"
#include "UI/Layouts.h"
#include <windows.h>

namespace MainWindow {
bool g_dragMode{false};

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

// fix obs mode
void SetObsMode(HWND hWnd, bool enabled) {
  int windowWidth{
      (int)(Layouts::activeLayout.windowWidth * MainWindow::getG_MainScale())};

  int windowHeight{
      (int)(Layouts::activeLayout.windowHeight * MainWindow::getG_MainScale())};

  // adding 30 pixles cuz of sysmenu
  int obsWindowHeight{
      ((int)(Layouts::activeLayout.windowHeight * MainWindow::getG_MainScale())) +
      (30)};

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
    exStyle |= ( WS_EX_TRANSPARENT | WS_EX_TOPMOST |
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
}; // namespace MainWindow

#include "SettingsWindow.h"
#include <minwindef.h>
#include <windows.h>
#include <winuser.h>

namespace SettingsWindow {
HWND g_hwnd{nullptr};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_CLOSE:
    Hide();
    return 0;
  case WM_DESTROY:
    g_hwnd = nullptr;
    return 0;
  }

  return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool create(HINSTANCE hInstance) {
  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = L"CatBoard Settings";

  RegisterClassEx(&wc);
  g_hwnd = CreateWindowEx(0, wc.lpszClassName, L"CatBoard Settings",
                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                          800, 600, nullptr, nullptr, hInstance, nullptr);
  return g_hwnd != nullptr;
}
void show(){
  ShowWindow(g_hwnd, SW_SHOW);
}

void hide(){
  ShowWindow(g_hwnd, SW_HIDE);
}

} // namespace SettingsWindow

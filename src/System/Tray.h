#pragma once

#include <windows.h>
struct AppState;

namespace Tray {
extern NOTIFYICONDATA g_nid;
NOTIFYICONDATA init(HWND hwnd, HINSTANCE hInstance);

void On_TrayWM_NOTIFYICON(WPARAM wParam, LPARAM lParam, HWND hWnd,
                          AppState &app);
void On_WM_COMMAND(WPARAM wParam, LPARAM lParam, HWND hWnd, AppState &app);

void updateIcon(HINSTANCE hInstance);
void remove();
} // namespace Tray

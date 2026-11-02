#include <windows.h>
struct AppState;
#ifndef SYSTEM_TRAY_H
#define SYSTEM_TRAY_H

namespace Tray {
extern NOTIFYICONDATA g_nid;
NOTIFYICONDATA init(HWND hwnd, HINSTANCE hInstance);

void On_TrayWM_NOTIFYICON(WPARAM wParam, LPARAM lParam, HWND hWnd,
                          AppState &app);
void On_WM_COMMAND(WPARAM wParam, LPARAM lParam, HWND hWnd, AppState &app);

} // namespace Tray

#endif // !SYSTEM_TRAY_H

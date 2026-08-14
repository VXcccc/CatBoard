#include <windows.h>
#ifndef SYSTEM_TRAY_H
#define SYSTEM_TRAY_H

namespace Tray {
extern NOTIFYICONDATA g_nid;
extern bool g_dragMode;
extern bool g_showSettings;


NOTIFYICONDATA init(HWND hwnd);

VOID APIENTRY DisplayContextMenu(HWND hWnd);

void On_TrayWM_NOTIFYICON(WPARAM wParam, LPARAM lParam, HWND hWnd);
void On_WM_COMMAND(WPARAM wParam, LPARAM lParam, HWND hWnd);

void RequestInteractive(HWND hwnd);
void ReleaseInteractive(HWND hwnd);
}

#endif // !TRAY_H

#include <windows.h>
#ifndef SYSTEM_TRAY_H
#define SYSTEM_TRAY_H

namespace Tray {
extern NOTIFYICONDATA g_nid;
extern bool g_dragMode;


NOTIFYICONDATA init(HWND hwnd);

VOID APIENTRY DisplayContextMenu(HWND hWnd);

void On_TrayWM_NOTIFYICON(WPARAM wParam, LPARAM lParam, HWND hWnd);
void On_WM_COMMAND(WPARAM wParam, LPARAM lParam, HWND hWnd);
void SetDragMode(HWND hWnd, bool enabled);
}

#endif // !TRAY_H

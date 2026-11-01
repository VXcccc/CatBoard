#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include "AppState.h"
#include <windows.h>
#include <d3d11.h>

namespace SettingsWindow {

bool Create(HINSTANCE hInstance, ID3D11Device* device, ID3D11DeviceContext* deviceContext);

void Show();
void Hide();
bool IsVisible();

void RenderFrame( AppState &app);

void Shutdown();

HWND GetHWND();
void ForceForeground(HWND hwnd);

} // namespace SettingsWindow

#endif // !SETTINGSWINDOW_H

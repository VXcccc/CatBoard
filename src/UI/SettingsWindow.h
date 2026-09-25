#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <windows.h>
#include <d3d11.h>

namespace SettingsWindow {

bool Create(HINSTANCE hInstance, ID3D11Device* device, ID3D11DeviceContext* deviceContext);

void Show();
void Hide();
bool IsVisible();

void RenderFrame();

void Shutdown();

HWND GetHWND();

} // namespace SettingsWindow

#endif // !SETTINGSWINDOW_H

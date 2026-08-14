#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <windows.h>
#include <d3d11.h>

namespace SettingsWindow {

// , ID3D11Device* device, ID3D11DeviceContext* deviceContext)
bool create(HINSTANCE hInstance);

void show();
void hide();
bool isVisible();

void renderFrame();

void shutdown();

HWND getHWND();

} // namespace SettingsWindow

#endif // !SETTINGSWINDOW_H

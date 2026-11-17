#pragma once

#include "AppState.h"
#include <windows.h>
#include <d3d11.h>

namespace SettingsWindow {

bool Create(HINSTANCE hInstance, ID3D11Device *device,
            ID3D11DeviceContext *deviceContext, AppState &app);

void Show();
void Hide();
bool IsVisible();
void toggleWindow();

void RenderFrame(AppState &app);

void Shutdown();

HWND GetHWND();
void ForceForeground(HWND hwnd);

} // namespace SettingsWindow


#pragma once
#include "imgui.h"
#include <d3d11.h>
#include <windows.h>

// gonna move it to another file ig

namespace MainWindow {

extern bool g_dragMode;
extern bool g_resizeMode;

void Show();
void Hide();
bool IsVisible();
void ApplyOverlayPosition();
void ApplyOverlaySize();

HWND getHWND();
float getG_MainScale();
ID3D11Device *get_g_pd3dDevice();

void setClearColor(const ImVec4 &clearColor);

void SetDragMode(HWND hWnd, bool enabled);
void SetResizeMode(HWND hWnd, bool enabled);
void SetObsMode(HWND hWnd, bool enabled);
} // namespace MainWindow

#pragma once
#include <windows.h>
#include "imgui.h"

namespace MainWindow {
extern bool g_dragMode;

void Show();
void Hide();
bool IsVisible();
void ApplyOverlayPosition();
void ApplyOverlaySize();

HWND getHWND();
float getG_MainScale();

void setClearColor(const ImVec4& clearColor);

void SetDragMode(HWND hWnd, bool enabled);
void SetObsMode(HWND hWnd, bool enabled);
} // namespace MainWindow

#pragma once
#include <windows.h>
#include "imgui.h"

namespace MainWindow {
void Show();
void Hide();
bool IsVisible();
void ApplyOverlayPosition();

HWND getHWND();
float getG_MainScale();

void setClearColor(const ImVec4& clearColor);
} // namespace MainWindow

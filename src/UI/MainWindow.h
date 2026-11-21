#pragma once
#include "imgui.h"
#include <d3d11.h>
#include <windows.h>

namespace Config {
struct Overlay;
} // namespace Config

struct AppState;

namespace MainWindow {

void Show();
void Hide();
bool IsVisible();
void toggleOverlay(AppState &app);

void ApplyOverlayPosition(const AppState &app);
void ApplyOverlaySize(const AppState &app);
void SetObsMode(HWND hWnd, bool enabled, const AppState &app);

HWND getHWND();
float getG_MainScale();
ID3D11Device *get_g_pd3dDevice();

void SetDragMode(HWND hWnd, bool enabled);
void toggleAutoHide(AppState &app);


bool overlayChanged(Config::Overlay &outPrevOv, const AppState &app);
ImVec4 getClearColor(const AppState &app);
} // namespace MainWindow

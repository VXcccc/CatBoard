#pragma once
#include "imgui.h"
#include <d3d11.h>
#include <windows.h>

namespace Config {
  struct Overlay;
	struct Settings;
}

namespace Layouts {
	struct Layout;
}

namespace MainWindow {

extern bool g_dragMode;
extern bool g_resizeMode;

void Show();
void Hide();
bool IsVisible();
void ApplyOverlayPosition(const Config::Settings &config,
                          const Layouts::Layout &layout);
void ApplyOverlaySize();

HWND getHWND();
float getG_MainScale();
ID3D11Device *get_g_pd3dDevice();

void setClearColor(const ImVec4 &clearColor);

void SetDragMode(HWND hWnd, bool enabled);
void SetResizeMode(HWND hWnd, bool enabled);
void SetObsMode(HWND hWnd, bool enabled);


bool overlayChanged(Config::Overlay &outPrevOv, const Config::Overlay &curOv);
} // namespace MainWindow

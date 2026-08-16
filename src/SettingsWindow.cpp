#include "SettingsWindow.h"
#include "Config.h"
#include "Resource.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <algorithm>
#include <cmath>
#include <dxgi.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

namespace SettingsWindow {

static HWND s_hwnd = nullptr;
static ID3D11Device *s_device = nullptr;
static ID3D11DeviceContext *s_deviceContext = nullptr;
static IDXGISwapChain *s_swapChain = nullptr;
static ID3D11RenderTargetView *s_rtv = nullptr;
static ImGuiContext *s_imguiContext = nullptr;
static bool s_visible = false;
static UINT s_resizeWidth = 0, s_resizeHeight = 0;

static void CreateRenderTarget() {
  ID3D11Texture2D *backBuffer = nullptr;
  s_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
  s_device->CreateRenderTargetView(backBuffer, nullptr, &s_rtv);
  backBuffer->Release();
}

static void CleanupRenderTarget() {
  if (s_rtv) {
    s_rtv->Release();
    s_rtv = nullptr;
  }
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam,
                                LPARAM lParam) {
  ImGuiContext *prev = ImGui::GetCurrentContext();
  ImGui::SetCurrentContext(s_imguiContext);

  LRESULT result = 0;
  bool handled = false;

  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
    handled = true;
    result = true;
  }

  if (!handled) {
    switch (msg) {
    case WM_SIZE:
      if (wParam != SIZE_MINIMIZED) {
        s_resizeWidth = LOWORD(lParam);
        s_resizeHeight = HIWORD(lParam);
      }
      handled = true;
      break;
    case WM_CLOSE:
      Hide();
      handled = true;
      break;
    case WM_DESTROY:
      handled = true;
      break;
    }
  }

  ImGui::SetCurrentContext(prev);

  if (handled)
    return result;
  return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool Create(HINSTANCE hInstance, ID3D11Device *device,
            ID3D11DeviceContext *deviceContext) {
  s_device = device;
  s_deviceContext = deviceContext;

  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc);
  wc.style = CS_CLASSDC;
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = L"CatBoardSettingsClass";
  wc.hIcon = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1),
                                          IMAGE_ICON, 0, 0,
                                          LR_DEFAULTSIZE | LR_SHARED));
  ::RegisterClassExW(&wc);

  s_hwnd = ::CreateWindowExW(0, wc.lpszClassName, L"CatBoard Settings",
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                             600, 400, nullptr, nullptr, hInstance, nullptr);
  if (!s_hwnd)
    return false;

  DXGI_SWAP_CHAIN_DESC sd{};
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = s_hwnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  IDXGIDevice *dxgiDevice = nullptr;
  s_device->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
  IDXGIAdapter *dxgiAdapter = nullptr;
  dxgiDevice->GetAdapter(&dxgiAdapter);
  IDXGIFactory *dxgiFactory = nullptr;
  dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

  HRESULT hr = dxgiFactory->CreateSwapChain(s_device, &sd, &s_swapChain);

  dxgiFactory->Release();
  dxgiAdapter->Release();
  dxgiDevice->Release();

  if (FAILED(hr))
    return false;

  CreateRenderTarget();

  s_imguiContext = ImGui::CreateContext();
  ImGuiContext *prev = ImGui::GetCurrentContext();
  ImGui::SetCurrentContext(s_imguiContext);

  ImGui::GetIO().IniFilename = nullptr; // .ini for settings
  ImGui::StyleColorsDark();
  ImGui_ImplWin32_Init(s_hwnd);
  ImGui_ImplDX11_Init(s_device, s_deviceContext);

  ImGui::SetCurrentContext(prev);

  ::ShowWindow(s_hwnd, SW_HIDE);
  return true;
}

void Show() {
  s_visible = true;
  ::ShowWindow(s_hwnd, SW_SHOW);
  ::SetForegroundWindow(s_hwnd);
}

void Hide() {
  s_visible = false;
  ::ShowWindow(s_hwnd, SW_HIDE);
}

bool IsVisible() { return s_visible; }

// Helper functions
int toColorRGB(float value) {
  value = std::clamp(value, 0.0f, 1.0f);
  return static_cast<int>(std::round(value * 255.0f));
}

float fromColorRGB(int value) {
  value = std::clamp(value, 0, 255);
  return static_cast<float>(value) / 255.0f;
}

Config::Color toConfColor(const ImVec4 &color) {
  return Config::Color{toColorRGB(color.x), toColorRGB(color.y),
                       toColorRGB(color.z), toColorRGB(color.w)};
}

ImVec4 fromConfColor(const Config::Color &color) {
  return ImVec4{fromColorRGB(color.r), fromColorRGB(color.g),
                fromColorRGB(color.b), fromColorRGB(color.o)};
}

void RenderFrame() {
  if (!s_visible)
    return;

  ImGuiContext *prev = ImGui::GetCurrentContext();
  ImGui::SetCurrentContext(s_imguiContext);

  if (s_resizeWidth != 0 && s_resizeHeight != 0) {
    CleanupRenderTarget();
    s_swapChain->ResizeBuffers(0, s_resizeWidth, s_resizeHeight,
                               DXGI_FORMAT_UNKNOWN, 0);
    s_resizeWidth = s_resizeHeight = 0;
    CreateRenderTarget();
  }

  // Main Loop Variables
  static ImVec4 idleFillCol{fromConfColor(Config::config.theme.idleFillCol)};
  static ImVec4 activeFillCol{
      fromConfColor(Config::config.theme.activeFillCol)};

  static ImVec4 idleTextCol{fromConfColor(Config::config.theme.idleTextCol)};
  static ImVec4 activeTextCol{
      fromConfColor(Config::config.theme.activeTextCol)};

  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  // gonna do otherwise ig destroy the win titlebar and use imgui but idk rn
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
  ImGui::Begin("SettingsRoot", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoSavedSettings);

  ImGui::ColorEdit4("Idle fill color", (float *)&idleFillCol);
  ImGui::ColorEdit4("Active fill color", (float *)&activeFillCol);

  ImGui::ColorEdit4("Idle text color", (float *)&idleTextCol);
  ImGui::ColorEdit4("Active text color", (float *)&activeTextCol);

  if (ImGui::Button("Save")) {
    Config::Save(Config::Settings{
        CATBOARD_VERSION,
        Config::Theme{toConfColor(idleFillCol), toConfColor(activeFillCol),
                      toConfColor(idleTextCol), toConfColor(activeTextCol)}});
  }

  ImGui::End();

  ImGui::Render();
  const float clearColor[4] = {0.10f, 0.10f, 0.12f, 1.0f};
  s_deviceContext->OMSetRenderTargets(1, &s_rtv, nullptr);
  s_deviceContext->ClearRenderTargetView(s_rtv, clearColor);
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  s_swapChain->Present(1, 0);

  ImGui::SetCurrentContext(prev);
}

void Shutdown() {
  if (!s_hwnd)
    return;

  ImGuiContext *prev = ImGui::GetCurrentContext();
  ImGui::SetCurrentContext(s_imguiContext);
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext(s_imguiContext);
  ImGui::SetCurrentContext(prev);

  CleanupRenderTarget();
  if (s_swapChain) {
    s_swapChain->Release();
    s_swapChain = nullptr;
  }

  ::DestroyWindow(s_hwnd);
  s_hwnd = nullptr;
}

} // namespace SettingsWindow

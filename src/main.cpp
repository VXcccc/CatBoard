// CatBoard
// Copyright (C) 2026 VXcccc
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License,
// or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// See LICENSE for the full license text.

#include "Paths.h"
#include <filesystem>
#include <libloaderapi.h>
#include <minwindef.h>
#ifndef DEBUG_CATBOARD
// #define DEBUG_CATBOARD
#endif

#include "AppState.h"
#include "Config.h"
#include "Constants.h"
#include "Fonts.h"
#include "Helpers.h"
#include "Images.h"
#include "Keyboard.h"
#include "Layouts.h"
#include "MainWindow.h"
#include "Mouse.h"
#include "SettingsWindow.h"
#include "Sound.h"
#include "Tray.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <conio.h>
#include <cstdlib>
#include <d3d11.h>
#include <dwmapi.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <hidclass.h>
#include <hidusage.h>
#include <iostream>
#include <shellapi.h>
#include <tchar.h>
#include <vector>
#include <windef.h>
#include <windows.h>
#include <winnt.h>
#include <winuser.h>

#include <strsafe.h>

#include <mmsystem.h>
// #pragma comment(lib, "winmm.lib")

#include <dcomp.h>
// #pragma comment(lib, "dcomp.lib")

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

// Data
static ID3D11Device *g_pd3dDevice = nullptr;
static ID3D11DeviceContext *g_pd3dDeviceContext = nullptr;
static IDXGISwapChain1 *g_pSwapChain = nullptr;
static bool g_SwapChainOccluded = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView *g_mainRenderTargetView = nullptr;
static HWND s_hwnd = nullptr;
static IDCompositionDevice *g_dcompDevice = nullptr;
static IDCompositionTarget *g_dcompTarget = nullptr;
static IDCompositionVisual *g_dcompVisual = nullptr;

static float g_main_scale{};

static HANDLE g_fpsTimer = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd, const Layouts::Layout &layout);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// user helper functions
POINT getOverlayPosition(int windowWidth, int windowHeight,
                         const Config::Settings &config);
bool RegisterRawMouseInput(HWND hwnd);
float getDeltaTime(const LARGE_INTEGER &freq, LARGE_INTEGER &outLastDeltaTime);
void fpsLimitter(
    const std::chrono::time_point<std::chrono::steady_clock> &frameStart,
    const Config::Settings &config);
void handleObsMode(AppState &app);
void initDebugConsole();

namespace MainWindow {
ID3D11Device *get_g_pd3dDevice() { return g_pd3dDevice; }

void Show() {
  ::ShowWindow(s_hwnd, SW_SHOW);
  ::SetForegroundWindow(s_hwnd);
}

void Hide() { ::ShowWindow(s_hwnd, SW_HIDE); }

bool IsVisible() { return s_hwnd && ::IsWindowVisible(s_hwnd); }

void ApplyOverlayPosition(const AppState &app) {
  if (!s_hwnd || app.dragMode) {
    return;
  }

  const auto &config{app.config};
  const auto &layout{app.layout};
  const auto &ovScale{config.overlay.scale};
  int windowWidth{(int)((layout.windowWidth * g_main_scale) * ovScale)};
  int windowHeight{(int)((layout.windowHeight * g_main_scale) * ovScale)};
  POINT pos{getOverlayPosition(windowWidth, windowHeight, config)};
  SetWindowPos(s_hwnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void ApplyOverlaySize(const AppState &app) {
  if (!s_hwnd)
    return;

  const auto &config{app.config};
  const auto &layout{app.layout};
  bool obsMode{config.misc.obsMode};
  const auto &ovScale{config.overlay.scale};
  int windowWidth{
      static_cast<int>((layout.windowWidth * g_main_scale) * ovScale)};

  int windowHeight{static_cast<int>(
      obsMode ? ((layout.windowHeight + 40.0f) * g_main_scale) * ovScale
              : (layout.windowHeight * g_main_scale) * ovScale)};

  SetWindowPos(s_hwnd, nullptr, 0, 0, windowWidth, windowHeight,
               SWP_NOMOVE | SWP_NOZORDER);
}

HWND getHWND() { return s_hwnd; }

float getG_MainScale() { return g_main_scale; }

ImVec4 getClearColor(const AppState &app) {
  return app.config.misc.obsMode
             ? Helper::fromConfigColor(app.config.theme.obsBackground)
             : Consts::defaultClearColor;
}

} // namespace MainWindow

// Main code
int main(int, char **) {
  // timeBeginPeriod(1);
#ifdef DEBUG_CATBOARD
  initDebugConsole();
#endif
  CreateMutexA(0, FALSE, "Local\\CATBOARD");
  if (GetLastError() == ERROR_ALREADY_EXISTS)
    Helper::fatalError(L"Program already running");

  // debug
  // initDebugConsole();

  // the state holding all active variables;
  AppState app;

  Config::LoadActiveConfig(app.config);
  app.previousObsMode = app.config.misc.obsMode;

  if (!std::filesystem::exists(Paths::getAssetsDir())) {
    Helper::errorMesBox(L"Missing assets folder");
    Paths::createAssetsDirectory();
  }

  Config::refreshConfigs();

  Layouts::refreshLayouts();

  if (!(Layouts::loadLayout(app.config.selectedLayout, app))) {
    Helper::errorMesBox(std::string("Missing layout: ") +
                        app.config.selectedLayout);
  }

  Sound::refreshSounds();

  if (!(Sound::initAudio())) {
    Helper::fatalError(L"Failed to init audio backends\nTry: vcpkg install "
                       L"libvorbis --triplet x64-mingw-dynamic");
  }

  if (!(Sound::loadSoundPack(app.config.soundPack.name, app.soundPack))) {
    Helper::errorMesBox(std::string("Missing sound pack: ") +
                        app.config.soundPack.name);
  }

  if (!(Images::Load(app.config.imagePack.name, app.imagePack))) {
    Helper::errorMesBox(std::string("Missing image pack: ") +
                        app.config.imagePack.name);
  }
  app.config.imagePack = app.imagePack;
  Images::refreshImagePacks();

  // Make process DPI aware and obtain main monitor scale
  ImGui_ImplWin32_EnableDpiAwareness();
  g_main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(
      ::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

  // Create application window
  WNDCLASSEXW wc = {sizeof(wc),
                    CS_CLASSDC,
                    WndProc,
                    0L,
                    0L,
                    GetModuleHandle(nullptr),
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    L"CatBoard Overlay",
                    nullptr};
  ::RegisterClassExW(&wc);

  int windowWidth{(int)(app.layout.windowWidth * g_main_scale)};
  int windowHeight{(int)(app.layout.windowHeight * g_main_scale)};

  POINT pos = getOverlayPosition(windowWidth, windowHeight, app.config);

  s_hwnd = ::CreateWindowExW(
      WS_EX_NOREDIRECTIONBITMAP | WS_EX_TRANSPARENT | WS_EX_TOPMOST |
          WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
      wc.lpszClassName, L"CatBoard Overlay", WS_POPUP, pos.x, pos.y,
      windowWidth, windowHeight, nullptr, nullptr, wc.hInstance, &app);
  // ::SetLayeredWindowAttributes(s_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

  if (!RegisterRawMouseInput(s_hwnd)) {
    Helper::fatalError(L"Failed to register mouse input");
  }

  // Initialize Direct3D
  if (!CreateDeviceD3D(s_hwnd, app.layout)) {
    CleanupDeviceD3D();
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 1;
  }

  // Show the window
  ::ShowWindow(s_hwnd, SW_SHOWDEFAULT);
  ::UpdateWindow(s_hwnd);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = nullptr;
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup scaling
  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(
      g_main_scale); // Bake a fixed style scale. (until we have a solution for
                     // dynamic style scaling, changing this requires resetting
                     // Style + calling this again)
  style.FontScaleDpi =
      g_main_scale; // Set initial font scale. (in docking branch: using
                    // io.ConfigDpiScaleFonts=true automatically overrides this
                    // for every window depending on the current monitor)

  // Setup Platform/Renderer backends
  ImGui_ImplWin32_Init(s_hwnd);
  ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

  if (!Fonts::refreshFonts()) {
    Helper::errorMesBox(L"Failed to get system fonts folder");
  };

  if (!(Fonts::loadFont(app.config.font, Fonts::overlayFont))) {
    if (!(Fonts::loadFont(Fonts::Font{}, Fonts::overlayFont))) {
      Helper::errorMesBox(
          L"Failed to load any fonts, check your fonts directory");
    }
  }

  SettingsWindow::Create(wc.hInstance, g_pd3dDevice, g_pd3dDeviceContext, app);

  g_fpsTimer = CreateWaitableTimerExW(nullptr, nullptr,
                                      CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
                                      TIMER_MODIFY_STATE | SYNCHRONIZE);

  if (!g_fpsTimer) {
    Helper::fatalError(L"Failed to create FPS timer");
  }

  // creating tray icon
  Tray::g_nid = Tray::init(s_hwnd, wc.hInstance);

  MainWindow::SetObsMode(s_hwnd, app.config.misc.obsMode, app);

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;

  // Main loop variables
  bool done = false;

  // prevKeyState holding previous frame state
  //
  // arrays only for layouts keys

  // arrays for full keyboard
  bool firstFrame = true;
  int prevScrollState{};
  Config::Overlay prevOverlay{};
  Fonts::Font lastAppliedFont{};

  // deltaTime
  LARGE_INTEGER freq;
  LARGE_INTEGER lastDeltaTime;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&lastDeltaTime);

  // debug
  // SettingsWindow::Show();

  while (!done) {
    const auto frameStart{std::chrono::steady_clock::now()};
    // Poll and handle messages (inputs, window resize, etc.)
    // See the WndProc() function below for our to dispatch events to the Win32
    // backend.

    Fonts::updateFont(lastAppliedFont, Fonts::overlayFont, app.config.font);

    handleObsMode(app);

    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
      if (msg.message == WM_QUIT)
        done = true;
    }
    if (done)
      break;

    // Handle window being minimized or screen locked
    if (g_SwapChainOccluded &&
        g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
      ::Sleep(10);
      continue;
    }
    g_SwapChainOccluded = false;

    // Handle window resize (we don't resize directly in the WM_SIZE handler)
    if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
      CleanupRenderTarget();
      g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight,
                                  DXGI_FORMAT_UNKNOWN, 0);
      g_ResizeWidth = g_ResizeHeight = 0;
      CreateRenderTarget();
    }

    float deltaTime{getDeltaTime(freq, lastDeltaTime)};

    bool overlayPropertiesChanged{MainWindow::overlayChanged(prevOverlay, app)};

    app.keyboard.pollFullKeyboard();

    // mouse poll
    app.mouse.consumePendingDelta();

    const Mouse::Type &curType{app.config.theme.mouse.style.type};

    const auto &ovScale{app.config.overlay.scale};

    if (Mouse::isTypeCircle(curType)) {
      app.mouse.updateMouseState(deltaTime, app.layout.mouse.radius,
                                 app.config.theme.mouse, ovScale);
    } else if (Mouse::isTypeRect(curType)) {
      app.mouse.updateMouseState(deltaTime, (app.layout.mouse.width / 2),
                                 (app.layout.mouse.height / 2),
                                 app.config.theme.mouse, ovScale);
    }

    if (app.config.theme.mouse.highlightM1)
      app.mouse.setM1Highlighted(app.keyboard.down[VK_LBUTTON]);
    else
      app.mouse.setM1Highlighted(false);

    app.mouse.updateScroll(deltaTime);
    app.mouse.updateTrail(deltaTime, app.config.theme.mouse, ovScale);

    bool anyKeyChanged{app.keyboard.anyChanged()};

    Keyboard::updateActive(app.layout, app.keyboard, app.mouse, app.config);

    bool scaleAnimating{Keyboard::updateScale(app.layout, app.keyboard,
                                              deltaTime, app.config.theme)};

    bool colorAnimating{Keyboard::updateKeyColors(app.layout, app.keyboard,
                                                  deltaTime, app.config.theme)};

    bool animationActive{scaleAnimating || colorAnimating};

    Binds::handleBinds(s_hwnd, app);

    if (app.config.soundPack.enable) {
      Keyboard::hadnleKeySounds(app.keyboard,
                                app.config.soundPack.layoutKeysOnlySound,
                                app.layout, app.soundPack);
    }

    int currentScrollState{app.mouse.getScrollDownTimes() +
                           app.mouse.getScrollUpTimes()};
    bool scrollActive{prevScrollState != currentScrollState};
    prevScrollState = currentScrollState;

    bool mouseIndicatorActive{
        std::abs(app.mouse.getOffsetX()) > app.mouse.getSensivity() ||
        std::abs(app.mouse.getOffsetY()) > app.mouse.getSensivity() ||
        scrollActive};

    bool needsRender{anyKeyChanged || firstFrame || mouseIndicatorActive ||
                     animationActive || overlayPropertiesChanged ||
                     (g_ResizeWidth != 0 && g_ResizeHeight != 0) ||
                     SettingsWindow::IsVisible()};

    if (!needsRender) {
      SettingsWindow::RenderFrame(app);
      fpsLimitter(frameStart, app.config);
      continue;
    }
    firstFrame = false;

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    Keyboard::renderKeyboard(app);

    ImVec2 mouseCenter(app.layout.mouse.centerX * ovScale,
                       app.layout.mouse.centerY * ovScale);
    if (app.config.theme.mouse.imagesDisplay) {
      Images::drawMouseImages(ImGui::GetBackgroundDrawList(), mouseCenter, app);
    }
    if (app.config.theme.mouse.enabled) {
      Mouse::renderMouseIndicator(mouseCenter,
                                  app.config.theme.mouse.style.type, app);
    }

    // debug
#ifdef DEBUG_CATBOARD
    // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
    //             1000.0f / io.Framerate, io.Framerate);
    ImGui::ShowDemoWindow();
#endif // !DEBUG_CATBOARD

    // ImGui::ShowDemoWindow();

    const auto frameEnd{std::chrono::steady_clock::now()};

    // Rendering
    ImGui::Render();
    const ImVec4 clearColor{MainWindow::getClearColor(app)};
    const float clear_color_with_alpha[4] = {
        clearColor.x * clearColor.w, clearColor.y * clearColor.w,
        clearColor.z * clearColor.w, clearColor.w};
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView,
                                            nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView,
                                               clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Present
    // HRESULT hr = g_pSwapChain->Present(1, 0); // Present with vsync
    // HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
    HRESULT hr = g_pSwapChain->Present(static_cast<UINT>(app.config.misc.vsync),
                                       0); // dynamic vsync choice

    SettingsWindow::RenderFrame(app);
    g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

    // todo move it to seperate function
    //  fps limiter
    fpsLimitter(frameStart, app.config);
  }

  // Cleanup
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  SettingsWindow::Shutdown();
  Sound::ShutdownAudio();
  ImGui::DestroyContext();

  CleanupDeviceD3D();
  ::DestroyWindow(s_hwnd);
  ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

  Tray::remove();

  // applying config before quit
  Config::Apply(app.config);

  if (g_fpsTimer) {
    CloseHandle(g_fpsTimer);
    g_fpsTimer = nullptr;
  }

  return 0;
}

// Main functions

// to do make it by toogle

void handleObsMode(AppState &app) {
  if (app.config.misc.obsMode != app.previousObsMode) {
    MainWindow::SetObsMode(s_hwnd, app.config.misc.obsMode, app);
    app.previousObsMode = app.config.misc.obsMode;
  }
}

// Helper functions
bool RegisterRawMouseInput(HWND hwnd) {
  RAWINPUTDEVICE rid{};
  rid.usUsagePage = HID_USAGE_GENERIC_POINTER;
  rid.usUsage = HID_USAGE_GENERIC_MOUSE;
  rid.dwFlags = RIDEV_INPUTSINK; // input without focus
  rid.hwndTarget = hwnd;

  return RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

float getDeltaTime(const LARGE_INTEGER &freq, LARGE_INTEGER &outLastDeltaTime) {
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  float dt = static_cast<float>(now.QuadPart - outLastDeltaTime.QuadPart) /
             freq.QuadPart;
  outLastDeltaTime = now;

  return dt;
}

// used clancker for this maybe not the best practice idk
void fpsLimitter(
    const std::chrono::time_point<std::chrono::steady_clock> &frameStart,
    const Config::Settings &config) {
  using Clock = std::chrono::steady_clock;
  using HundredNanoseconds =
      std::chrono::duration<long long, std::ratio<1, 10000000>>;

  const double fps{static_cast<double>(config.misc.fps)};

  if (fps <= 0.0 || g_fpsTimer == nullptr) {
    return;
  }

  const auto targetFrame{std::chrono::duration_cast<Clock::duration>(
      std::chrono::duration<double>(1.0 / fps))};

  const auto targetTime{frameStart + targetFrame};
  const auto now{Clock::now()};

  if (now >= targetTime) {
    return;
  }

  const auto remaining{targetTime - now};

  const auto remaining100ns{
      std::chrono::duration_cast<HundredNanoseconds>(remaining).count()};

  if (remaining100ns <= 0) {
    return;
  }

  LARGE_INTEGER dueTime{};
  dueTime.QuadPart = -remaining100ns;

  if (!SetWaitableTimer(g_fpsTimer, &dueTime, 0, nullptr, nullptr, FALSE)) {
    return;
  }

  WaitForSingleObject(g_fpsTimer, INFINITE);
}

POINT getOverlayPosition(int windowWidth, int windowHeight,
                         const Config::Settings &config) {
  POINT savedPos{config.overlay.x, config.overlay.y};

  // todo maybe rework this shi
  int centerX{savedPos.x + windowWidth / 2};
  int centerY{savedPos.y + windowHeight / 2};

  HMONITOR monitor{
      MonitorFromPoint(POINT{centerX, centerY}, MONITOR_DEFAULTTONEAREST)};

  MONITORINFO mi{};
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(monitor, &mi);

  int minX = mi.rcWork.left;
  int minY = mi.rcWork.top;
  int maxX = (std::max)(minX, static_cast<int>(mi.rcWork.right - windowWidth));
  int maxY =
      (std::max)(minY, static_cast<int>(mi.rcWork.bottom - windowHeight));

  POINT result;
  result.x = std::clamp(static_cast<int>(savedPos.x), minX, maxX);
  result.y = std::clamp(static_cast<int>(savedPos.y), minY, maxY);
  return result;
}

void initDebugConsole() {
  if (GetConsoleWindow() == nullptr && !AllocConsole())
    return;

  FILE *stream{};

  if (freopen_s(&stream, "CONIN$", "r", stdin) != 0)
    return;

  if (freopen_s(&stream, "CONOUT$", "w", stdout) != 0)
    return;

  if (freopen_s(&stream, "CONOUT$", "w", stderr) != 0)
    return;

  std::printf("Debugging Window:\n");
  std::fflush(stdout);
}

bool CreateDeviceD3D(HWND hWnd, const Layouts::Layout &layout) {
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2]{D3D_FEATURE_LEVEL_11_0,
                                               D3D_FEATURE_LEVEL_10_0};

  D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                    D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevelArray, 2,
                    D3D11_SDK_VERSION, &g_pd3dDevice, &featureLevel,
                    &g_pd3dDeviceContext);

  IDXGIDevice *dxgiDevice{nullptr};
  g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
  IDXGIAdapter *dxgiAdapter{nullptr};
  dxgiDevice->GetAdapter(&dxgiAdapter);
  IDXGIFactory2 *dxgiFactory{nullptr};
  dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

  DXGI_SWAP_CHAIN_DESC1 sd{};
  sd.Width = static_cast<UINT>(layout.windowWidth * g_main_scale);
  sd.Height = static_cast<UINT>(layout.windowHeight * g_main_scale);
  sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

  sd.BufferCount = 2;
  sd.SampleDesc.Count = 1;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

  sd.Scaling = DXGI_SCALING_STRETCH;
  sd.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

  dxgiFactory->CreateSwapChainForComposition(g_pd3dDevice, &sd, nullptr,
                                             &g_pSwapChain);

  DCompositionCreateDevice(dxgiDevice, IID_PPV_ARGS(&g_dcompDevice));
  g_dcompDevice->CreateTargetForHwnd(hWnd, TRUE, &g_dcompTarget);
  g_dcompDevice->CreateVisual(&g_dcompVisual);
  g_dcompVisual->SetContent(g_pSwapChain);
  g_dcompTarget->SetRoot(g_dcompVisual);
  g_dcompDevice->Commit();

  dxgiFactory->Release();
  dxgiAdapter->Release();
  dxgiDevice->Release();

  CreateRenderTarget();
  return true;
}

void CleanupDeviceD3D() {
  CleanupRenderTarget();
  if (g_pSwapChain) {
    g_pSwapChain->Release();
    g_pSwapChain = nullptr;
  }
  if (g_pd3dDeviceContext) {
    g_pd3dDeviceContext->Release();
    g_pd3dDeviceContext = nullptr;
  }
  if (g_pd3dDevice) {
    g_pd3dDevice->Release();
    g_pd3dDevice = nullptr;
  }
}

void CreateRenderTarget() {
  ID3D11Texture2D *pBackBuffer;
  g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr,
                                       &g_mainRenderTargetView);
  pBackBuffer->Release();
}

void CleanupRenderTarget() {
  if (g_mainRenderTargetView) {
    g_mainRenderTargetView->Release();
    g_mainRenderTargetView = nullptr;
  }
}
void WM_RawMouseInputHandle(LPARAM lParam, Mouse::State &mouseState) {
  UINT dataSize;
  GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dataSize,
                  sizeof(RAWINPUTHEADER));

  if (dataSize > 0) {
    std::vector<BYTE> buffer(dataSize);
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &dataSize,
                        sizeof(RAWINPUTHEADER)) == dataSize) {
      RAWINPUT *raw = (RAWINPUT *)buffer.data();
      if (raw->header.dwType == RIM_TYPEMOUSE) {
        LONG rawDeltaX = raw->data.mouse.lLastX;
        LONG rawDeltaY = raw->data.mouse.lLastY;
        mouseState.addRawDelta(rawDeltaX, rawDeltaY);
      }
      if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) {
        short wheelDelta{static_cast<short>(raw->data.mouse.usButtonData)};
        mouseState.registerScroll(wheelDelta);
      }
    }
  }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse and io.WantCaptureKeyboard flags to tell
// if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to
// your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data
// to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them
// from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (msg == WM_NCCREATE) {
    const auto *createInfo{reinterpret_cast<const CREATESTRUCTW *>(lParam)};
    SetWindowLongPtrW(hWnd, GWLP_USERDATA,
                      reinterpret_cast<LONG_PTR>(createInfo->lpCreateParams));
  }
  auto *app{
      reinterpret_cast<AppState *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA))};

  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  switch (msg) {
  case WM_INPUT: {
    if (app)
      WM_RawMouseInputHandle(lParam, app->mouse);
    break;
  }
  case WM_SIZE:
    if (wParam == SIZE_MINIMIZED)
      return 0;
    g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
    g_ResizeHeight = (UINT)HIWORD(lParam);
    return 0;
  case WM_SYSCOMMAND:
    if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
      return 0;
    break;
  // handle tray icon
  case Consts::WM_TRAYICON:
    if (app)
      Tray::On_TrayWM_NOTIFYICON(wParam, lParam, hWnd, *app);
    return 0;
  // handle menu
  case WM_COMMAND:
    if (app)
      Tray::On_WM_COMMAND(wParam, lParam, hWnd, *app);
    return 0;
  case WM_NCHITTEST:
    if (app && app->dragMode) {
      LRESULT position = DefWindowProc(hWnd, msg, wParam, lParam);
      return position == HTCLIENT ? HTCAPTION : position;
    }
    break;
  case WM_EXITSIZEMOVE: {
    // RECT rect;
    // GetWindowRect(hWnd, &rect);
    // app.config.overlay.x = rect.left;
    // app.config.overlay.y = rect.top;
    if (app)
      Config::Apply(app->config);
    return 0;
  }
  case WM_DESTROY:
    if (app->config.misc.saveOnClose) {
      Config::Apply(app->config);
    }
    Tray::remove();
    ::PostQuitMessage(0);
    return 0;
  }
  return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

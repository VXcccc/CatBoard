// Dear ImGui: standalone example application for Windows API + DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/
// folder).
// - Introduction, links and more at the top of imgui.cpp
//

#include "Config.h"
#include "Constants.h"
#include "FontHandle.h"
#include "Helpers.h"
#include "Layouts.h"
#include "MainWindow.h"
#include "MouseState.h"
#include "SettingsWindow.h"
#include "Sound.h"
#include "SystemTray.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <InputHandle.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <conio.h>
#include <cstdlib>
#include <d3d11.h>
#include <dxgi.h>
#include <shellapi.h>
#include <tchar.h>
#include <thread>
#include <windef.h>
#include <windows.h>
#include <winnt.h>
#include <winuser.h>

#include <strsafe.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// Data
static ID3D11Device *g_pd3dDevice = nullptr;
static ID3D11DeviceContext *g_pd3dDeviceContext = nullptr;
static IDXGISwapChain *g_pSwapChain = nullptr;
static bool g_SwapChainOccluded = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView *g_mainRenderTargetView = nullptr;
static bool s_visible = true;
static HWND s_hwnd = nullptr;

static float g_main_scale{};

// Forward declarations of main functions
void renderKeyboard(const Layouts::Layout &layout,
                    const std::array<bool, 256> &currentKeysDown,
                    const std::array<bool, 256> &currentFullKeysDown);
bool detectAnyKeyChanged(const Layouts::Layout &layout,
                         const std::array<bool, 256> &prevKeyState,
                         std::array<bool, 256> &outCurrentDown);
void commitKeyState(const Layouts::Layout &layout,
                    const std::array<bool, 256> &currentKeysDown,
                    std::array<bool, 256> &outPrevKeyState);

void handleKeySound(const Layouts::Layout &layout,
                    const std::array<bool, 256> &prevKeyState,
                    const std::array<bool, 256> &currentDown);

void handleKeySoundFull(const std::array<bool, 256> &prevKeyState,
                        const std::array<bool, 256> &currentKeysDown);

void pollFullKeyboard(std::array<bool, 256> &outCurrentKeysDown);

void handleObsMode();

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

POINT getOverlayPosition(int windowWidth, int windowHeight);

bool RegisterRawMouseInput(HWND hwnd);

float getDeltaTime(const LARGE_INTEGER &freq, LARGE_INTEGER &outLastDeltaTime);

namespace MainWindow {
static ImVec4 s_clearColor{Consts::defaultClearColor};

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

void ApplyOverlayPosition() {
  int windowWidth = (int)(Layouts::activeLayout.windowWidth * g_main_scale);
  int windowHeight = (int)(Layouts::activeLayout.windowHeight * g_main_scale);
  POINT pos = getOverlayPosition(windowWidth, windowHeight);
  SetWindowPos(s_hwnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void ApplyOverlaySize() {
  int windowWidth =
      static_cast<int>(Layouts::activeLayout.windowWidth * g_main_scale);

  int windowHeight =
      static_cast<int>(Layouts::activeLayout.windowHeight * g_main_scale);

  SetWindowPos(s_hwnd, nullptr, 0, 0, windowWidth, windowHeight,
               SWP_NOMOVE | SWP_NOZORDER);
}

HWND getHWND() { return s_hwnd; }

float getG_MainScale() { return g_main_scale; }

void setClearColor(const ImVec4 &clearColor) { s_clearColor = clearColor; }

} // namespace MainWindow

// Main code
int main(int, char **) {
  timeBeginPeriod(1);
  Config::Load();

  if (!(Layouts::installDefaultLayouts())) {
    Helper::fatalError(L"Failed to install default layouts");
  }

  Layouts::refreshLayouts();

  if (!(Layouts::loadLayout(Config::activeConfig.selectedLayout))) {
    Helper::fatalError(L"Failed to load keyboard layout");
  }

  if (!(Sound::installDefaultSoundPacks())) {
    Helper::fatalError(L"Failed to install default sound packs");
  }

  Sound::refreshSounds();

  if (!(Sound::initAudio())) {
    Helper::fatalError(L"Failed to init audio backends\nTry: vcpkg install "
                       L"libvorbis --triplet x64-mingw-dynamic");
  }

  if (!(Sound::loadSoundPack(Config::activeConfig.soundPack.name))) {
    Helper::fatalError(L"Failed to load sound pack");
  }

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

  int windowWidth{(int)(Layouts::activeLayout.windowWidth * g_main_scale)};
  int windowHeight{(int)(Layouts::activeLayout.windowHeight * g_main_scale)};

  POINT pos = getOverlayPosition(windowWidth, windowHeight);

  s_hwnd = ::CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST |
                                 WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                             wc.lpszClassName, L"CatBoard Overlay", WS_POPUP,
                             pos.x, pos.y, windowWidth, windowHeight, nullptr,
                             nullptr, wc.hInstance, nullptr);

  ::SetLayeredWindowAttributes(s_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

  if (!RegisterRawMouseInput(s_hwnd)) {
    Helper::fatalError(L"Failed to register mouse input");
  }

  // Initialize Direct3D
  if (!CreateDeviceD3D(s_hwnd)) {
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

  if (!(Fonts::installDefaultFonts())) {
    Helper::fatalError(L"Failed to install default fonts");
  }
  Fonts::refreshFonts();
  if (!(Fonts::loadFont(Config::activeConfig.font))) {
    Helper::fatalError(L"Failed to load font");
  }

  SettingsWindow::Create(wc.hInstance, g_pd3dDevice, g_pd3dDeviceContext);

  // Load Fonts
  // fontCfg.FontDataOwnedByAtlas = false;

  // io.Fonts->AddFontFromMemoryTTF(Courier_new, sizeof(Courier_new), 20.0f,
  // &fontCfg);

  // IM_ASSERT(Fonts::activeFont != nullptr);

  // - If fonts are not explicitly loaded, Dear ImGui will select an embedded
  // font: either AddFontDefaultVector() or AddFontDefaultBitmap().
  //   This selection is based on (style.FontSizeBase * style.FontScaleMain *
  //   style.FontScaleDpi) reaching a small threshold.
  // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - If a file cannot be loaded, AddFont functions will return a nullptr.
  // Please handle those errors in your code (e.g. use an assertion, display an
  // error and quit).
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType
  // for higher quality font rendering.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // style.FontSizeBase = 20.0f;
  // io.Fonts->AddFontDefaultVector();
  // io.Fonts->AddFontDefaultBitmap();
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
  // IM_ASSERT(font != nullptr);

  // creating tray icon
  Tray::g_nid = Tray::init(s_hwnd, wc.hInstance);

  MainWindow::SetObsMode(s_hwnd, Config::activeConfig.misc.obsMode);

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;

  // Main loop
  bool done = false;

  // prevKeyState holding previous frame state
  //
  // arrays only for layouts keys
  static std::array<bool, 256> prevKeyState{};
  static std::array<bool, 256> currentKeysDown{};

  // arrays for full keyboard
  static std::array<bool, 256> prevFullKeyState{};
  static std::array<bool, 256> currentFullKeysDown{};
  static bool firstFrame = true;

  // deltaTime
  LARGE_INTEGER freq;
  LARGE_INTEGER lastDeltaTime;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&lastDeltaTime);

  while (!done) {
    const auto frameStart{std::chrono::steady_clock::now()};
    // Poll and handle messages (inputs, window resize, etc.)
    // See the WndProc() function below for our to dispatch events to the Win32
    // backend.

    static Fonts::Font lastAppliedFont{};
    if (Config::activeConfig.font.name != lastAppliedFont.name ||
        Config::activeConfig.font.sizePixels != lastAppliedFont.sizePixels) {
      if (Fonts::loadFont(Config::activeConfig.font)) {
        lastAppliedFont = Config::activeConfig.font;
      }
    }

    handleObsMode();

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

    bool anyKeyChanged{detectAnyKeyChanged(Layouts::activeLayout, prevKeyState,
                                           currentKeysDown)};

    pollFullKeyboard(currentFullKeysDown);

    // mouse poll
    Mouse::g_mouseState.consumePendingDelta();
    float deltaTime{getDeltaTime(freq, lastDeltaTime)};
    Mouse::g_mouseState.updateMouseState(
        deltaTime, Config::activeConfig.theme.mouseTheme.radius);
    // Mouse::g_mouseState.updateTrail(deltaTime, 0.5f);

    if (Input::g_bindJustRecorded) {
      prevFullKeyState = currentFullKeysDown;
      Input::g_bindJustRecorded = false;
    } else {
      Input::handleBinds(prevFullKeyState, currentFullKeysDown, s_hwnd);
    }

    if (Config::activeConfig.soundPack.enable) {
      if (Config::activeConfig.soundPack.layoutKeysOnlySound) {
        handleKeySound(Layouts::activeLayout, prevKeyState, currentKeysDown);
      } else {
        handleKeySoundFull(prevFullKeyState, currentFullKeysDown);
      }
    }

    prevFullKeyState = currentFullKeysDown;

    bool mouseIndicatorActive{std::abs(Mouse::g_mouseState.getOffsetX()) >
                                  Mouse::g_mouseState.getSensivity() ||
                              std::abs(Mouse::g_mouseState.getOffsetY()) >
                                  Mouse::g_mouseState.getSensivity()};

    bool needsRender{anyKeyChanged || firstFrame || mouseIndicatorActive ||
                     (g_ResizeWidth != 0 && g_ResizeHeight != 0) ||
                     SettingsWindow::IsVisible()};

    if (!needsRender) {
      SettingsWindow::RenderFrame();
      ::Sleep(1);
      continue;
    }
    firstFrame = false;

    commitKeyState(Layouts::activeLayout, currentKeysDown, prevKeyState);

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    renderKeyboard(Layouts::activeLayout, currentKeysDown, currentFullKeysDown);

    if (Config::activeConfig.theme.mouseTheme.enabled) {
      Mouse::renderMouseIndicator(
          ImVec2(Layouts::activeLayout.mouse.mouseCenterX,
                 Layouts::activeLayout.mouse.mouseCenterY),
          Config::activeConfig.theme.mouseTheme.style.type);
    }

    // debug
    //  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
    //              1000.0f / io.Framerate, io.Framerate);

    // Rendering
    ImGui::Render();
    const float clear_color_with_alpha[4] = {
        MainWindow::s_clearColor.x * MainWindow::s_clearColor.w,
        MainWindow::s_clearColor.y * MainWindow::s_clearColor.w,
        MainWindow::s_clearColor.z * MainWindow::s_clearColor.w,
        MainWindow::s_clearColor.w};
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView,
                                            nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView,
                                               clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Present
    // HRESULT hr = g_pSwapChain->Present(1, 0); // Present with vsync
    // HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
    HRESULT hr = g_pSwapChain->Present(
        static_cast<UINT>(Config::activeConfig.misc.vsync),
        0); // dynamic vsync choice

    SettingsWindow::RenderFrame();
    g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

    // todo move it to seperate function
    //  fps limiter
    const auto frameEnd{std::chrono::steady_clock::now()};

    const double frameTime{
        std::chrono::duration<double>(frameEnd - frameStart).count()};

    double targetFrameTime{1.0 /
                           static_cast<double>(Config::activeConfig.misc.fps)};

    if (frameTime < targetFrameTime) {
      std::this_thread::sleep_for(
          std::chrono::duration<double>(targetFrameTime - frameTime));
    }
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

  Shell_NotifyIcon(NIM_DELETE, &Tray::g_nid);

  // applying config before quit
  Config::Apply(Config::activeConfig);

  timeEndPeriod(1);
  return 0;
}

// Main functions

void renderKeyboard(
    const Layouts::Layout &layout, const std::array<bool, 256> &currentKeysDown,
    const std::array<bool, 256>
        &currentFullKeysDown) { // todo maybe remove currentFullKeysDown
  ImDrawList *draw{ImGui::GetBackgroundDrawList()};
  ImGui::PushFont(Fonts::activeFont);

  for (const auto &key : Layouts::activeLayout.keys) {
    if (key.keyCode != Consts::MouseInputVKCODE) {
      bool down{currentKeysDown[key.keyCode]};
      // bool down{(GetAsyncKeyState(key.keyCode) & 0x8000) != 0};

      // i did first if statement for peoples who have caps binded to smth, you
      // can remove it if you want to show caps lock toogle state without
      // changing buttons labels
      if (Config::activeConfig.font.letterCase.options.capsLockDisplay) {
        if (key.keyCode == VK_CAPITAL) {
          down = (GetKeyState(VK_CAPITAL) & 1) != 0;
        }
      }

      ImVec2 topLeft{key.x, key.y};
      ImVec2 bottomRight{key.x + key.w, key.y + key.h};

      ImU32 fill{down ? Config::ToImU32(Config::activeConfig.theme.activeFill)
                      : Config::ToImU32(Config::activeConfig.theme.idleFill)};

      ImU32 textColor{
          down ? Config::ToImU32(Config::activeConfig.theme.activeText)
               : Config::ToImU32(Config::activeConfig.theme
                                     .idleText)}; // oposite the fill color

      float rounding{Config::activeConfig.theme.keyRounding};

      draw->AddRectFilled(topLeft, bottomRight, fill, rounding);
      if (Config::activeConfig.theme.keyOutline.enable) {
        ImU32 outlineColor{
            Config::ToImU32(Config::activeConfig.theme.keyOutline.color)};
        float thickness{Config::activeConfig.theme.keyOutline.thickness};
        draw->AddRect(topLeft, bottomRight, outlineColor, rounding, 0,
                      thickness);
      }
      // outline

      std::string displayLabel{Fonts::handleLetterCase(
          currentFullKeysDown, Config::activeConfig.font, key)};

      ImVec2 textSize{ImGui::CalcTextSize(key.label.c_str())};
      ImVec2 textPos{key.x + (key.w - textSize.x) * 0.5f,
                     key.y + (key.h - textSize.y) * 0.5f};
      draw->AddText(textPos, textColor, displayLabel.c_str());
    }
  }
  ImGui::PopFont();
}

// to do make it by toogle

bool detectAnyKeyChanged(const Layouts::Layout &layout,
                         const std::array<bool, 256> &prevKeyState,
                         std::array<bool, 256> &outCurrentKeysDown) {
  bool anyKeyChanged = false;
  for (const auto &key : layout.keys) {
    bool down{(GetAsyncKeyState(key.keyCode) & 0x8000) != 0};
    outCurrentKeysDown[key.keyCode] = down;
    if (down != prevKeyState[key.keyCode]) {
      anyKeyChanged = true;
    }
  }

  return anyKeyChanged;
}

void pollFullKeyboard(std::array<bool, 256> &outCurrentKeysDown) {
  for (int vk{0}; vk < 256; ++vk) {
    outCurrentKeysDown[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
  }
}

void commitKeyState(const Layouts::Layout &layout,
                    const std::array<bool, 256> &currentKeysDown,
                    std::array<bool, 256> &outPrevKeyState) {
  for (const auto &key : layout.keys) {
    outPrevKeyState[key.keyCode] = currentKeysDown[key.keyCode];
  }
}

// sound functions
void handleKeySound(const Layouts::Layout &layout,
                    const std::array<bool, 256> &prevKeyState,
                    const std::array<bool, 256> &currentKeysDown) {
  for (const auto &key : layout.keys) {
    bool down{currentKeysDown[key.keyCode]};
    if (down != prevKeyState[key.keyCode]) {
      Sound::playKey(key.keyCode, down);
    }
  }
}

void handleKeySoundFull(const std::array<bool, 256> &prevKeyState,
                        const std::array<bool, 256> &currentKeysDown) {
  for (const auto &[vk, def] : Sound::activePack.KeyDefByVK) {
    bool down{currentKeysDown[vk]};
    if (down != prevKeyState[vk]) {
      Sound::playKey(vk, down);
    }
  }
}

void handleObsMode() {
  static bool lastObsModeCondition{Config::activeConfig.misc.obsMode};
  if (Config::activeConfig.misc.obsMode != lastObsModeCondition) {
    MainWindow::SetObsMode(s_hwnd, Config::activeConfig.misc.obsMode);
    lastObsModeCondition = Config::activeConfig.misc.obsMode;
  }

  if (Config::activeConfig.misc.obsMode) {
    MainWindow::setClearColor(
        Helper::fromConfigColor(Config::activeConfig.theme.obsBackground));
  } else {
    MainWindow::setClearColor(Consts::defaultClearColor);
  }
}

// Helper functions
bool RegisterRawMouseInput(HWND hwnd) {
  RAWINPUTDEVICE rid{};
  rid.usUsagePage = 0x01;        // Generic Desktop Controls
  rid.usUsage = 0x02;            // Mouse
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

POINT getOverlayPosition(int windowWidth, int windowHeight) {
  POINT savedPos{Config::activeConfig.overlayPos.x,
                 Config::activeConfig.overlayPos.y};

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

bool CreateDeviceD3D(HWND hWnd) {
  // Setup swap chain
  // This is a basic setup. Optimally could use e.g.
  // DXGI_SWAP_EFFECT_FLIP_DISCARD and handle fullscreen mode differently. See
  // #8979 for suggestions.
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT createDeviceFlags = 0;
  // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_0,
  };
  HRESULT res = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
      featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
      &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
  if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software
                                     // driver if hardware is not available.
    res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
        &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
  if (res != S_OK)
    return false;

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
void WM_RawMouseInputHandle(LPARAM lParam) {
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
        Mouse::g_mouseState.addRawDelta(rawDeltaX, rawDeltaY);
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
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell
// if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to
// your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data
// to your main application, or clear/overwrite your copy of the keyboard
// data. Generally you may always pass all inputs to dear imgui, and hide them
// from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  switch (msg) {
  case WM_INPUT: {
    WM_RawMouseInputHandle(lParam);
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
    Tray::On_TrayWM_NOTIFYICON(wParam, lParam, hWnd);
    return 0;
  // handle menu
  case WM_COMMAND:
    Tray::On_WM_COMMAND(wParam, lParam, hWnd);
    return 0;
  case WM_NCHITTEST:
    if (MainWindow::g_dragMode) {
      LRESULT position = DefWindowProc(hWnd, msg, wParam, lParam);
      return position == HTCLIENT ? HTCAPTION : position;
    }
    break;
  case WM_EXITSIZEMOVE: {
    RECT rect;
    GetWindowRect(hWnd, &rect);
    Config::activeConfig.overlayPos.x = rect.left;
    Config::activeConfig.overlayPos.y = rect.top;
    Config::Apply(Config::activeConfig);
    return 0;
  }
  case WM_DESTROY:
    ::PostQuitMessage(0);
    return 0;
  }
  return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

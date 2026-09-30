#include "UI/SettingsWindow.h"
#include "Assets/Resource/ResourceConstants.h"
#include "Config/Config.h"
#include "Config/ConfigJson.h"
#include "Config/Constants.h"
#include "Input/Hotkeys.h"
#include "Input/Mouse.h"
#include "Media/Fonts.h"
#include "Media/Images.h"
#include "Media/Sound.h"
#include "System/Helpers.h"
#include "System/Paths.h"
#include "System/Tray.h"
#include "UI/Layouts.h"
#include "UI/MainWindow.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <dwmapi.h>
#include <dxgi.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <winuser.h>

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

// Forward declarations

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

HWND GetHWND() { return s_hwnd; }

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
                             900, 600, nullptr, nullptr, hInstance, nullptr);

  BOOL useDarkMode{true};
  DwmSetWindowAttribute(s_hwnd,
                        DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
                        &useDarkMode, sizeof(useDarkMode));
  RECT rc;

  GetWindowRect(s_hwnd, &rc);

  // centering the settings window
  int xWindowPos{(GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2};
  int yWindowPos{(GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2};

  SetWindowPos(s_hwnd, 0, xWindowPos, yWindowPos, 0, 0,
               SWP_NOZORDER | SWP_NOSIZE);

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

  ImGuiIO &io = ImGui::GetIO();

  Fonts::Font settingsFont{"JetBrainsMonoNerdFontMono-Bold", 17.0f};
  ImFont *font = io.Fonts->AddFontFromFileTTF(
      Paths::getFontPath(settingsFont.name).string().c_str(),
      settingsFont.sizePixels);

  io.IniFilename = nullptr; // .ini for settings
  ImGui::StyleColorsDark();
  ImGui_ImplWin32_Init(s_hwnd);
  ImGui_ImplDX11_Init(s_device, s_deviceContext);

  ImGui::SetCurrentContext(prev);

  ::ShowWindow(s_hwnd, SW_HIDE);
  return true;
}

// focuses on the window when oppening it
void ForceForeground(HWND hwnd) {
  HWND foreground{GetForegroundWindow()};
  DWORD foregroundThreadId{GetWindowThreadProcessId(foreground, nullptr)};
  DWORD currentThreadId{GetCurrentThreadId()};

  if (foregroundThreadId != currentThreadId) {
    AttachThreadInput(foregroundThreadId, currentThreadId, TRUE);
    SetForegroundWindow(hwnd);
    AttachThreadInput(foregroundThreadId, currentThreadId, FALSE);
  } else {
    SetForegroundWindow(hwnd);
  }
}

void Show() {
  s_visible = true;

  if (IsIconic(s_hwnd)) {
    ::ShowWindow(s_hwnd, SW_RESTORE);
  } else {
    ::ShowWindow(s_hwnd, SW_SHOW);
  }
  ::SetWindowPos(s_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

  ForceForeground(s_hwnd);
  // ::SetForegroundWindow(s_hwnd);
  // ::SetFocus(s_hwnd);

  ::SetWindowPos(s_hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void Hide() {
  s_visible = false;
  ::ShowWindow(s_hwnd, SW_HIDE);
}

bool IsVisible() { return s_visible; }

// Tabs handling
enum class SettingsTab {
  Visuals,
  Layouts,
  Sounds,
  Misc,
  Binds,
  Configs,
  max_SettingsTabs
};

struct Tab {
  const char *label;
  SettingsTab type;
};

std::array<Tab, static_cast<std::size_t>(SettingsTab::max_SettingsTabs)> tabs{
    {{"Visuals", SettingsTab::Visuals},
     {"Layouts", SettingsTab::Layouts},
     {"Sounds", SettingsTab::Sounds},
     {"Misc", SettingsTab::Misc},
     {"Binds", SettingsTab::Binds},
     {"Configs", SettingsTab::Configs}}};

static_assert(tabs.size() ==
                  static_cast<std::size_t>(SettingsTab::max_SettingsTabs),
              "tabs array is out of sync with SettingsTab enum");

// Helper functions

struct SettingsEditState {
  Config::Settings previous;

  Config::Theme theme;

  std::string selectedLayout;

  Config::SoundPackSelection soundPack;
  int volumePercent{};

  Binds::BindsArray binds{};

  Fonts::Font font{};

  int letterCaseMode{}; // converting to enum in new settings

  Config::Misc misc{};

  std::string configName{};

  Images::Pack imagePack{};

  // not writing this to the config
  std::size_t currentLayout{};
  std::size_t currentConfig{};
  std::size_t currentSound{};
  std::size_t currentFont{};
  std::size_t currentMouseStyle{};
  std::size_t currentImagePack{};
  std::size_t currentTrailStyle{};

  std::string configNameInput{};
  std::string configNameError;
  std::string layoutDeleteError{};

  bool showLayoutEditor{};

  std::vector<Config::Settings> undoSettings{};
  std::vector<Config::Settings> redoSettings{};
};

void restoreFromConfig(SettingsEditState &state,
                       const Config::Settings &prevSettings) {
  state.theme = prevSettings.theme;

  state.selectedLayout = prevSettings.selectedLayout;

  state.soundPack = prevSettings.soundPack;
  state.volumePercent =
      static_cast<int>(std::round(prevSettings.soundPack.volume * 100.0f));

  state.binds = prevSettings.binds;

  state.font = prevSettings.font;

  state.misc = prevSettings.misc;

  state.configName = prevSettings.configName;

  state.imagePack = prevSettings.imagePack;

  Layouts::loadLayout(prevSettings.selectedLayout);
  Sound::loadSoundPack(prevSettings.soundPack.name);
  Sound::setMasterVolume(prevSettings.soundPack.volume);
}

Config::Settings buildSettings(const SettingsEditState &state) {
  return {Consts::CATBOARD_VERSION,
          state.theme,
          state.selectedLayout,
          Config::SoundPackSelection{
              state.soundPack.name, state.soundPack.enable,
              state.soundPack.layoutKeysOnlySound,
              static_cast<float>(state.volumePercent) / 100.0f},
          state.binds,
          Config::activeConfig.overlayPos,
          state.font,
          state.misc,
          state.configName,
          state.imagePack};
}

void pushUndo(SettingsEditState &state, const Config::Settings &snapshot) {
  if (state.undoSettings.size() >= Consts::MAX_UNDO_STEPS) {
    state.undoSettings.erase(state.undoSettings.begin());
  }
  state.undoSettings.push_back(snapshot);
  state.redoSettings.clear();
}

void clearUndo(SettingsEditState &state) {
  state.undoSettings.clear();
  state.redoSettings.clear();
}

void undo(SettingsEditState &state) {
  if (state.undoSettings.empty())
    return;

  state.redoSettings.push_back(buildSettings(state));

  Config::Settings prevSettings{state.undoSettings.back()};
  state.undoSettings.pop_back();

  Config::activeConfig = prevSettings;
  restoreFromConfig(state, prevSettings);

  // added this cuz for some reason its loses focus after pressing ctrl z
  ForceForeground(s_hwnd);
}

void redo(SettingsEditState &state) {
  if (state.redoSettings.empty())
    return;

  state.undoSettings.push_back(buildSettings(state));

  Config::Settings nextSettings{state.redoSettings.back()};
  state.redoSettings.pop_back();

  Config::activeConfig = nextSettings;
  restoreFromConfig(state, nextSettings);
  ForceForeground(s_hwnd);
}

void handleUndoRedoInput(SettingsEditState &state,
                         Config::Settings &lastCommitted,
                         SettingsTab currentTab) {
  if (!ImGui::GetIO().WantTextInput && currentTab != SettingsTab::Layouts) {
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z)) {
      redo(state);
      lastCommitted = buildSettings(state);
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z)) {
      undo(state);
      lastCommitted = buildSettings(state);
    }
  }
}

void validateCurrentIndex(std::size_t &index, std::size_t size) {
  if (index >= size) {
    index = size == 0 ? 0 : size - 1;
  }
}

std::size_t findIndexByName(const std::vector<std::string> &vec,
                            std::string_view name) {

  const auto it{std::find(vec.begin(), vec.end(), name)};
  if (it == vec.end()) {
    return 0;
  }
  return static_cast<std::size_t>(std::distance(vec.begin(), it));
}

// Helper draw functions

void drawGenericCombo(
    const char *label, const std::vector<std::string> &items,
    const char *emptyLabel, std::size_t &outCurrentItem,
    std::function<void(std::size_t, const std::string &)> onSelect) {
  validateCurrentIndex(outCurrentItem, std::size(items));

  const char *preview{items.empty() ? emptyLabel
                                    : items[outCurrentItem].c_str()};

  if (ImGui::BeginCombo(label, preview)) {
    for (std::size_t i{0}; i < items.size(); ++i) {
      bool isSelected{outCurrentItem == i};
      if (ImGui::Selectable(items[i].c_str(), isSelected)) {
        onSelect(i, items[i]);
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
}

void drawLayoutCombo(SettingsEditState &state) {
  drawGenericCombo("Layout", Layouts::layouts, "No layouts",
                   state.currentLayout,
                   [&state](std::size_t i, const std::string &name) {
                     if (Layouts::loadLayout(name)) {
                       state.currentLayout = i;
                       state.selectedLayout = name;
                     }
                   });
}

void drawImagePackCombo(SettingsEditState &state) {
  drawGenericCombo("Image Pack", Images::packs, "No image packs",
                   state.currentImagePack,
                   [&state](std::size_t i, const std::string &name) {
                     if (Images::Load(name)) {
                       state.currentImagePack = i;
                       state.imagePack.name = name;
                     }
                   });
}

void drawFontCombo(SettingsEditState &state) {
  drawGenericCombo("Font", Fonts::fonts, "No fonts", state.currentFont,
                   [&state](std::size_t i, const std::string &name) {
                     state.currentFont = i;
                     state.font.name = name;
                   });
}

void drawConfigCombo(SettingsEditState &state) {
  drawGenericCombo("Config", Config::configs, "No configs", state.currentConfig,
                   [&state](std::size_t i, const std::string &name) {
                     state.currentConfig = i;
                     state.configName = name;
                   });
}

void drawSoundCombo(SettingsEditState &state) {
  drawGenericCombo("Sound", Sound::sounds, "No sounds", state.currentSound,
                   [&state](std::size_t i, const std::string &name) {
                     if (Sound::loadSoundPack(name)) {
                       state.currentSound = i;
                       state.soundPack.name = name;
                     }
                   });
}

void drawMouseStyleCommbo(SettingsEditState &state,
                          const std::vector<std::string> &mouseStyleNames) {
  drawGenericCombo("Mouse style", mouseStyleNames, "No mouse styles",
                   state.currentMouseStyle,
                   [&state](std::size_t i, const std::string &name) {
                     state.currentMouseStyle = i;
                     state.theme.mouse.style.type = static_cast<Mouse::Type>(i);
                     state.theme.mouse.style.name = name;
                   });
}
void drawTrailStyleCombo(SettingsEditState &state,
                         const std::vector<std::string> &trailStyleNames) {
  drawGenericCombo("Trail style", trailStyleNames, "No trail styles",
                   state.currentTrailStyle,
                   [&state](std::size_t i, const std::string &name) {
                     state.currentTrailStyle = i;
                     state.theme.mouse.trail.style.type =
                         static_cast<Mouse::TrailType>(i);
                     state.theme.mouse.trail.style.name = name;
                   });
}

// Draw sections functions
void drawKeyboardColors(SettingsEditState &state) {
  Helper::ColorEdit("Idle fill color", state.theme.idleFill);
  Helper::ColorEdit("Active fill color", state.theme.activeFill);
  Helper::ColorEdit("Idle text color", state.theme.idleText);
  Helper::ColorEdit("Active text color", state.theme.activeText);
}

void drawKeyboardSection(SettingsEditState &state) {
  ImGui::SeparatorText("Keyboard");

  drawKeyboardColors(state);

  ImGui::SliderFloat("Rounding", &state.theme.keyRounding, 0.0f, 20.0f, "%.2f");
  state.theme.keyRounding = std::clamp(state.theme.keyRounding, 0.0f, 1000.0f);

  ImGui::Checkbox("Animations", &state.theme.animation);
  if (state.theme.animation) {
    ImGui::SliderFloat("Press scale", &state.theme.pressScale, 0.0f, 3.0f,
                       "%.2f");
    ImGui::SliderInt("Animation speed", &state.theme.animationSpeed, 0, 1000);
  }
  ImGui::Checkbox("Enable Outline", &state.theme.keyOutline.enable);
  if (state.theme.keyOutline.enable) {
    Helper::ColorEdit("Idle outline color", state.theme.keyOutline.idleColor);
    Helper::ColorEdit("Active outline color",
                      state.theme.keyOutline.activeColor);

    ImGui::SliderFloat("Thickness", &state.theme.keyOutline.thickness, 0.0f,
                       20.0f, "%.2f");
    // you can change this value if you want more thickness lol
    state.theme.keyOutline.thickness =
        std::clamp(state.theme.keyOutline.thickness, 0.0f, 20.0f);
  }
}

void drawMouseSection(SettingsEditState &state) {
  auto &mouse{state.theme.mouse};
  auto &trail{mouse.trail};
  bool circleType{mouse.style.type == Mouse::Type::CatBoard ||
                  mouse.style.type == Mouse::Type::NohBoard};
  bool rectType{mouse.style.type == Mouse::Type::Wrap ||
                mouse.style.type == Mouse::Type::Pan};

  ImGui::SeparatorText("Mouse");

  ImGui::Checkbox("Enable mouse input display", &state.theme.mouse.enabled);

  if (state.theme.mouse.enabled) {
    drawMouseStyleCommbo(state, Helper::extractNames(Mouse::mouseStyles));

    Helper::ColorEdit("Outer color", mouse.outer);
    Helper::ColorEdit("Inner color", mouse.inner);
    Helper::ColorEdit("Fill color", mouse.fill);

    ImGui::Checkbox("Invert indicator color", &state.theme.mouse.lerpColor);

    ImGui::Checkbox("Highlight m1", &mouse.highlightM1);
    if (mouse.highlightM1) {
      Helper::ColorEdit("Highlight color", mouse.highlightColor);

      ImGui::SliderFloat("Highlight scale", &mouse.higlightScale, 0.0f, 3.0f);
      mouse.higlightScale = std::clamp(mouse.higlightScale, 0.0f, 10.0f);
    }

    if (circleType)
      ImGui::SliderFloat("Radius", &state.theme.mouse.radius, 0.0f, 100.0f,
                         "%.2f");

    if (rectType) {
      ImGui::SliderFloat("Width", &state.theme.mouse.width, 0.0f, 1920.0f);
      ImGui::SliderFloat("Height", &state.theme.mouse.height, 0.0f, 1920.0f);
    }

    ImGui::SliderFloat("Indicator radius", &state.theme.mouse.smallRadius, 0.0f,
                       100.0f, "%.2f");

    ImGui::SliderFloat("Outer thickness", &state.theme.mouse.outerThickness,
                       0.0f, 100.0f, "%.2f");

    ImGui::SliderFloat("Sensitivity", &state.theme.mouse.sensitivity, 0.0f,
                       10.0f, "%.2f");
    if (circleType)
      ImGui::SliderFloat("Decay rate", &state.theme.mouse.decayRate, 0.0f,
                         100.0f, "%.2f");

    ImGui::SliderFloat("Fade time", &state.theme.mouse.fadeTime, 0.0f, 3.0f);

    if (rectType)
      ImGui::SliderFloat("Outline rounding", &state.theme.mouse.rounding, 0.0f,
                         20.0f, "%.2f");
    state.theme.mouse.rounding =
        std::clamp(state.theme.mouse.rounding, 0.0f, 1000.0f);

    // wip
    ImGui::Checkbox("Trail", &state.theme.mouse.trail.enable);
    if (state.theme.mouse.trail.enable) {
      drawTrailStyleCombo(state, Helper::extractNames(Mouse::trailStyles));

      ImGui::SliderFloat("Trail duration", &state.theme.mouse.trail.duration,
                         0.0f, 3.0f);

      ImGui::SliderFloat("Min width", &mouse.trail.minWidth, 0.0f, 10.0f,
                         "%.2f");
      ImGui::SliderFloat("Max width", &mouse.trail.maxWidth, 0.0f, 30.0f,
                         "%.2f");

      if (mouse.trail.style.type == Mouse::TrailType::Catmull) {
        ImGui::SliderFloat("Max length", &mouse.trail.maxLength, 0.0f, 1000.0f,
                           "%.2f");
        ImGui::SliderInt("Subdivisions", &mouse.trail.subdivisions, 1.0f,
                         30.0f);
        mouse.trail.subdivisions = std::clamp(mouse.trail.subdivisions, 1, 100);
      }

      Helper::ColorEdit("Trail color", trail.color);
    }
  }
}

void drawImageSection(SettingsEditState &state) {
  ImGui::SeparatorText("Image");

  drawImagePackCombo(state);

  if (ImGui::Button("Open images folder")) {
    Paths::openImagesDirectory();
  }

  ImGui::SameLine();

  if (ImGui::Button("Refresh image packs")) {
    Images::refreshImagePacks();
  }

  ImGui::Checkbox("Mouse display", &state.theme.mouse.imagesDisplay);
  if (state.theme.mouse.imagesDisplay) {
    ImGui::SliderInt("Mouse opacity", &state.theme.mouse.imageOpacity, 0, 255);
    state.theme.mouse.imageOpacity =
        std::clamp(state.theme.mouse.imageOpacity, 0, 255);

    ImGui::DragFloat("Offset X", &state.theme.mouse.imageOffsetX);
    ImGui::DragFloat("Offset Y", &state.theme.mouse.imageOffsetY);
  }

  static int prevIdleAlpha{};
  static int prevActiveAlpha{};
  if (ImGui::Checkbox("Keys display", &state.theme.keysImagesDisplay)) {
    if (state.theme.keysImagesDisplay) {
      prevIdleAlpha = state.theme.idleFill[3];
      prevActiveAlpha = state.theme.activeFill[3];

      state.theme.idleFill[3] = 0;
      state.theme.activeFill[3] = 0;
    } else {
      state.theme.idleFill[3] = prevIdleAlpha;
      state.theme.activeFill[3] = prevActiveAlpha;
    }
  }

  if (state.theme.keysImagesDisplay) {
    ImGui::SliderInt("Keys opacity", &state.theme.keysImageOpacity, 0, 255);
    state.theme.keysImageOpacity =
        std::clamp(state.theme.keysImageOpacity, 0, 255);
  }
}

void drawFontSection(SettingsEditState &state) {
  ImGui::SeparatorText("Font");

  drawFontCombo(state);

  ImGui::SliderFloat("Font size", &state.font.sizePixels, 0.0f, 100.0f, "%.2f");
  state.font.sizePixels = std::clamp(state.font.sizePixels, 1.0f, 300.0f);

  if (ImGui::Button("Open fonts folder")) {
    Paths::openFontsDirectory();
  }

  ImGui::SameLine();

  if (ImGui::Button("Refresh fonts")) {
    Fonts::refreshFonts();
    validateCurrentIndex(state.currentFont, std::size(Fonts::fonts));
  }

  ImGui::Text("Capitalization of keys: ");

  ImGui::RadioButton("Show all buttons defaulted", &state.letterCaseMode,
                     static_cast<int>(Fonts::LetterCaseMode::defaultMode));
  ImGui::RadioButton("Show all buttons capitalized", &state.letterCaseMode,
                     static_cast<int>(Fonts::LetterCaseMode::allCapital));
  ImGui::RadioButton("Show all buttons lower-case", &state.letterCaseMode,
                     static_cast<int>(Fonts::LetterCaseMode::allLower));
  state.font.letterCase.mode =
      static_cast<Fonts::LetterCaseMode>(state.letterCaseMode);

  ImGui::Text("Options: ");
  ImGui::Checkbox("Caps Lock applying for keys",
                  &state.font.letterCase.options.capsLockDisplay);
  ImGui::Checkbox("Follow shift for caps Lock insensitive keys",
                  &state.font.letterCase.options.capsInsensitiveKeys);
  ImGui::Checkbox("Follow shift for caps Lock sensitive keys",
                  &state.font.letterCase.options.capsSensitiveKeys);
}

// Draw. functions

void drawVisualsTab(SettingsEditState &state) {
  drawKeyboardSection(state);
  drawMouseSection(state);

  drawImageSection(state);

  drawFontSection(state);
}

void drawLayoutsTab(SettingsEditState &state,
                    Layouts::LayoutEditState &layoutEditState) {
  drawLayoutCombo(state);

  if (ImGui::Button("Open layout folder")) {
    Paths::openLayoutDirectory();
  }

  ImGui::SameLine();

  if (ImGui::Button("Refresh layouts")) {
    Layouts::refreshLayouts();
    validateCurrentIndex(state.currentLayout, std::size(Layouts::layouts));
  }

  ImGui::SameLine();

  if (ImGui::Button("Delete layout")) {
    if (state.selectedLayout != Layouts::defaultLayoutName &&
        Layouts::layouts.size() > 1) {
      state.layoutDeleteError.clear();

      Layouts::deleteLayout(state.selectedLayout);
      Layouts::refreshLayouts();
      validateCurrentIndex(state.currentLayout, std::size(Layouts::layouts));
    } else {
      state.layoutDeleteError = "You can not delete default or last layout";
    }
  }

  if (!state.layoutDeleteError.empty()) {
    Helper::drawErrorInSettings(state.layoutDeleteError);
  }

  ImGui::SeparatorText("Editor");

  ImGui::Checkbox("Show layout editor", &state.showLayoutEditor);
  if (state.showLayoutEditor) {
    Layouts::drawLayoutEditorTab(layoutEditState);
  }
}

void drawMiscTab(SettingsEditState &state) {
  ImGui::Checkbox("Enable V-Sync", &state.misc.vsync);
  ImGui::Checkbox("Enable ObsMode", &state.misc.obsMode);

  if (state.misc.obsMode) {
    Helper::ColorEdit("Background Color", state.theme.obsBackground);
  }

  ImGui::SliderInt("FPS", &state.misc.fps, 5, 360);
  state.misc.fps = std::clamp(state.misc.fps, 5, 2000);
}

void drawBindsTab(SettingsEditState &state) {
  for (auto &bind : state.binds) {
    ImGui::Text("%s", bind.bindName.c_str());
    ImGui::SameLine();
    if (ImGui::Button(bind.keyName.c_str())) {
      Binds::recordBind(bind);
    }
  }
}

void drawSoundsTab(SettingsEditState &state) {
  ImGui::Checkbox("Enable asmr sounds", &state.soundPack.enable);
  if (state.soundPack.enable) {
    ImGui::Checkbox("Sounds only for keys in layout",
                    &state.soundPack.layoutKeysOnlySound);

    if (ImGui::SliderInt("Volume", &state.volumePercent, 0, 100)) {
      Sound::setMasterVolume(static_cast<float>(state.volumePercent / 100.0f));
    }
    state.volumePercent = std::clamp(state.volumePercent, 0, 100);

    drawSoundCombo(state);

    if (ImGui::Button("Open sounds folder")) {
      Paths::openSoundsDirectory();
    }

    ImGui::SameLine();

    if (ImGui::Button("Refresh sounds")) {
      Sound::refreshSounds();
      validateCurrentIndex(state.currentSound, std::size(Sound::sounds));
    }
  }
}

void drawConfigsTab(SettingsEditState &state) {
  drawConfigCombo(state);

  if (ImGui::Button("Save config")) {
    Config::saveConfig(Config::configs[state.currentConfig],
                       buildSettings(state));
  }

  ImGui::SameLine();

  if (ImGui::Button("Load config")) {
    if (Config::loadConfig(Config::configs[state.currentConfig])) {
      restoreFromConfig(state, Config::activeConfig);

      if (!Layouts::loadLayout(Config::activeConfig.selectedLayout)) {
        Helper::errorMesBox(L"Failed to load keyboard layout from config");
      }

      if (!Sound::loadSoundPack(Config::activeConfig.soundPack.name)) {
        Helper::errorMesBox(L"Failed to load sound pack from config");
      }

      MainWindow::ApplyOverlayPosition();

      state.previous = Config::activeConfig;
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("Delete config")) {
    Config::deleteConfig(Config::configs[state.currentConfig]);
    Config::refreshConfigs();
    validateCurrentIndex(state.currentConfig, std::size(Config::configs));
  }

  ImGui::SameLine();

  if (ImGui::Button("Refresh configs")) {
    Config::refreshConfigs();
    validateCurrentIndex(state.currentConfig, std::size(Config::configs));
  }

  ImGui::InputText("##Config name", &state.configNameInput);
  ImGui::SameLine();
  if (ImGui::Button("Create config")) {
    state.configNameError =
        Helper::getFileCreateError(Config::createConfig(state.configNameInput));
    if (state.configNameError.empty()) {
      Config::refreshConfigs();
    }
  }

  if (!state.configNameError.empty()) {
    Helper::drawErrorInSettings(state.configNameError);
  }

  if (ImGui::Button("Open config folder")) {
    Paths::openConfigDirectory();
  }
}

void drawApplyCancel(SettingsEditState &state,
                     const Config::Settings &newSettings) {

  if (ImGui::Button("Apply")) {
    Config::Apply(newSettings);
    state.previous = newSettings;
  }
  ImGui::SameLine();

  if (ImGui::Button("Cancel")) {
    Config::activeConfig = state.previous;
    restoreFromConfig(state, state.previous);
  }
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
  // Draw functions

  // Main Loop Variables
  static bool wasVisibleLastFrame = false;
  bool visibleNow = SettingsWindow::IsVisible();

  static SettingsEditState editState{};
  static Layouts::LayoutEditState layoutEditState{};
  static SettingsTab currentTab = SettingsTab::Visuals;

  static Config::Settings lastCommitted{};

  if (visibleNow && !wasVisibleLastFrame) {
    Layouts::refreshLayouts();
    Config::refreshConfigs();
    Sound::refreshSounds();

    editState.previous = Config::activeConfig;
    restoreFromConfig(editState, editState.previous);
    lastCommitted = editState.previous;

    editState.currentConfig =
        findIndexByName(Config::configs, Config::activeConfig.configName);
    editState.currentLayout =
        findIndexByName(Layouts::layouts, Config::activeConfig.selectedLayout);
    editState.currentSound =
        findIndexByName(Sound::sounds, Config::activeConfig.soundPack.name);
    editState.currentFont =
        findIndexByName(Fonts::fonts, Config::activeConfig.font.name);
    editState.currentImagePack =
        findIndexByName(Images::packs, Config::activeConfig.imagePack.name);
    editState.currentMouseStyle =
        findIndexByName(Helper::extractNames(Mouse::mouseStyles),
                        Config::activeConfig.theme.mouse.style.name);
    editState.currentTrailStyle =
        findIndexByName(Helper::extractNames(Mouse::trailStyles),
                        Config::activeConfig.theme.mouse.trail.style.name);
  }
  wasVisibleLastFrame = visibleNow;

  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  // maybe destroy the win titlebar and use imgui but idk rn
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
  ImGui::Begin("SettingsRoot", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_HorizontalScrollbar);

  for (size_t i{0}; i < std::size(tabs); ++i) {
    if (ImGui::Button(tabs[i].label,
                      ImVec2(ImGui::GetWindowSize().x / std::size(tabs) - 12.0f,
                             25.0f))) // todo remove magic numbers
      currentTab = tabs[i].type;

    if (i + 1 < std::size(tabs))
      ImGui::SameLine();
  }

  handleUndoRedoInput(editState, lastCommitted, currentTab);

  switch (currentTab) {
  case SettingsTab::Visuals:
    drawVisualsTab(editState);
    break;
  case SettingsTab::Layouts:
    drawLayoutsTab(editState, layoutEditState);
    break;
  case SettingsTab::Sounds:
    drawSoundsTab(editState);
    break;
  case SettingsTab::Misc:
    drawMiscTab(editState);
    break;
  case SettingsTab::Binds:
    drawBindsTab(editState);
    break;
  case SettingsTab::Configs:
    drawConfigsTab(editState);
    break;
  default:
    break;
  }

  Config::Settings newSettings = buildSettings(editState);
  // Passing new settings but dont saving them
  Config::activeConfig = newSettings;

  bool settingsChanged{nlohmann::json(newSettings).dump() !=
                       nlohmann::json(lastCommitted).dump()};
  if (settingsChanged && !ImGui::IsAnyItemActive() &&
      currentTab != SettingsTab::Layouts) {
    pushUndo(editState, lastCommitted);
    lastCommitted = newSettings;
  }

  if (currentTab == SettingsTab::Visuals || currentTab == SettingsTab::Sounds ||
      currentTab == SettingsTab::Binds || currentTab == SettingsTab::Misc ||
      currentTab == SettingsTab::Layouts) {
    drawApplyCancel(editState, newSettings);
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

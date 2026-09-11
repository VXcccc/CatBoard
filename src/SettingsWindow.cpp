#include "SettingsWindow.h"
#include "Config.h"
#include "Constants.h"
#include "Helpers.h"
#include "InputHandle.h"
#include "Paths.h"
#include "Sound.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include <array>
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

// Tabs handling
enum class SettingsTab { Visuals, Sounds, Binds, Configs, max_SettingsTabs };

struct Tab {
  const char *label;
  SettingsTab type;
};

std::array<Tab, static_cast<std::size_t>(SettingsTab::max_SettingsTabs)> tabs{
    {{"Visuals", SettingsTab::Visuals},
     {"Sounds", SettingsTab::Sounds},
     {"Binds", SettingsTab::Binds},
     {"Configs", SettingsTab::Configs}}};

static_assert(tabs.size() ==
                  static_cast<std::size_t>(SettingsTab::max_SettingsTabs),
              "tabs array is out of sync with SettingsTab enum");

// Helper functions

struct SettingsEditState {
  Config::Settings previous;

  ImVec4 idleFillCol;
  ImVec4 activeFillCol;
  ImVec4 idleTextCol;
  ImVec4 activeTextCol;

  float rounding{};

  Config::Outline outline;

  std::string selectedLayout;

  Config::SoundPackSelection selectedSoundPack;
  int volumePercent{};

  Input::BindsArray binds{};

  std::size_t currentLayout{};
  std::size_t currentConfig{};
  std::size_t currentSound{};

  std::string configNameInput{};
  std::string configNameError;
};

int toColorRGB(float value) {
  value = std::clamp(value, 0.0f, 1.0f);
  return static_cast<int>(std::round(value * 255.0f));
}

float fromColorRGB(int value) {
  value = std::clamp(value, 0, 255);
  return static_cast<float>(value) / 255.0f;
}

Config::Theme::ColorRGBA toConfigColor(const ImVec4 &color) {
  return Config::Theme::ColorRGBA{toColorRGB(color.x), toColorRGB(color.y),
                                  toColorRGB(color.z), toColorRGB(color.w)};
}

ImVec4 fromConfigColor(const Config::Theme::ColorRGBA &color) {
  return ImVec4{fromColorRGB(color[0]), fromColorRGB(color[1]),
                fromColorRGB(color[2]), fromColorRGB(color[3])};
}

void restoreFromConfig(SettingsEditState &state,
                       const Config::Settings &prevSettings) {
  state.idleFillCol = fromConfigColor(prevSettings.theme.idleFill);
  state.activeFillCol = fromConfigColor(prevSettings.theme.activeFill);
  state.idleTextCol = fromConfigColor(prevSettings.theme.idleText);
  state.activeTextCol = fromConfigColor(prevSettings.theme.activeText);
  state.rounding = prevSettings.theme.rounding;

  state.outline = prevSettings.theme.outline;

  state.selectedLayout = prevSettings.selectedLayout;

  state.selectedSoundPack = prevSettings.soundPack;
  state.volumePercent =
      static_cast<int>(std::round(prevSettings.soundPack.volume * 100.0f));
  state.binds = prevSettings.binds;

  Input::loadLayout(prevSettings.selectedLayout);
  Sound::loadSoundPack(prevSettings.soundPack.name);
  Sound::setMasterVolume(prevSettings.soundPack.volume);
}

void handleCreateConfig(SettingsEditState &state) {
  const auto result = Config::createConfig(state.configNameInput);

  switch (result) {
  case Config::CreateConfigResult::Success:
    state.configNameError.clear();
    break;
  case Config::CreateConfigResult::EmptyName:
    state.configNameError = "Config name cannot be empty";
    break;
  case Config::CreateConfigResult::NameTooLong:
    state.configNameError = "Config name is too long";
    break;
  case Config::CreateConfigResult::AlreadyExists:
    state.configNameError = "Config already exists";
    break;
  case Config::CreateConfigResult::DirectoryError:
    state.configNameError = "Could not create config directory";
    break;
  case Config::CreateConfigResult::FileError:
    state.configNameError = "Could not create config file";
    break;
  default:
    break;
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

void drawLayoutCombo(std::size_t &outCurrentItem, SettingsEditState &state) {
  validateCurrentIndex(outCurrentItem, std::size(Input::layouts));

  const char *preview{Input::layouts.empty()
                          ? "No layouts"
                          : Input::layouts[outCurrentItem].c_str()};
  if (ImGui::BeginCombo("Layout", preview)) {
    for (std::size_t i{0}; i < std::size(Input::layouts); ++i) {
      bool isSelected{outCurrentItem == i};
      if (ImGui::Selectable(Input::layouts[i].c_str(), isSelected)) {
        if (Input::loadLayout(Input::layouts[i])) {
          outCurrentItem = i;
          state.selectedLayout = Input::layouts[i];
        }
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
}

void drawConfigCombo(std::size_t &outCurrentItem) {
  validateCurrentIndex(outCurrentItem, std::size(Config::configs));

  const char *preview{Config::configs.empty()
                          ? "No configs"
                          : Config::configs[outCurrentItem].c_str()};

  if (ImGui::BeginCombo("Config", preview)) {
    for (std::size_t i{0}; i < std::size(Config::configs); ++i) {
      bool isSelected{(outCurrentItem == i)};
      if (ImGui::Selectable(Config::configs[i].c_str(), isSelected)) {
        outCurrentItem = i;
      }
      if (isSelected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

void drawSoundCombo(std::size_t &outCurrentItem, SettingsEditState &state) {
  validateCurrentIndex(outCurrentItem, std::size(Sound::sounds));

  const char *preview{Sound::sounds.empty()
                          ? "No sounds"
                          : Sound::sounds[outCurrentItem].c_str()};
  if (ImGui::BeginCombo("Sound", preview)) {
    for (std::size_t i{0}; i < std::size(Sound::sounds); ++i) {
      bool isSelected{outCurrentItem == i};
      if (ImGui::Selectable(Sound::sounds[i].c_str(), isSelected)) {
        if (Sound::loadSoundPack(Sound::sounds[i])) {
          outCurrentItem = i;

          state.selectedSoundPack.name = Sound::sounds[i];
        }
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
}

// Draw functions

void drawVisualsTab(SettingsEditState &state) {
  ImGui::ColorEdit4("Idle fill color", &state.idleFillCol.x);
  ImGui::ColorEdit4("Active fill color", &state.activeFillCol.x);

  ImGui::ColorEdit4("Idle text color", &state.idleTextCol.x);
  ImGui::ColorEdit4("Active text color", &state.activeTextCol.x);

  ImGui::SliderFloat("Rounding", &state.rounding, 0.0f, 20.0f, "%.2f");
  state.rounding = std::clamp(state.rounding, 0.0f, 20.0f);

  drawLayoutCombo(state.currentLayout, state);

  if (ImGui::Button("Open layout folder")) {
    Paths::openLayoutDirectory();
  }

  ImGui::SameLine();

  if (ImGui::Button("Refresh layouts")) {
    Input::refreshLayouts();
    validateCurrentIndex(state.currentLayout, std::size(Input::layouts));
  }

  ImGui::Checkbox("Enable outline", &state.outline.enable);
  if (state.outline.enable) {
    ImVec4 outlineColor{fromConfigColor(state.outline.color)};
    if (ImGui::ColorEdit4("Outline color", &outlineColor.x)) {
      state.outline.color = toConfigColor(outlineColor);
    };

    ImGui::SliderFloat("Thickness", &state.outline.thickness, 0.0f, 20.0f,
                       "%.2f");
    // you can change this value if you want more thickness lol
    state.outline.thickness = std::clamp(state.outline.thickness, 0.0f, 20.0f);
  }
}

void drawBindsTab(SettingsEditState &state) {
  for (auto &bind : state.binds) {
    ImGui::Text("%s", bind.bindName.c_str());
    ImGui::SameLine();
    if(ImGui::Button(bind.keyName.c_str())){
      Input::recordBind(bind);
    }
  }
}

void drawSoundsTab(SettingsEditState &state) {

  ImGui::Checkbox("Enable asmr sounds", &state.selectedSoundPack.enable);
  if (state.selectedSoundPack.enable) {
    ImGui::Checkbox("Sounds only for keys in layout",
                    &state.selectedSoundPack.layoutKeysOnlySound);

    if (ImGui::SliderInt("Volume", &state.volumePercent, 0, 100)) {
      Sound::setMasterVolume(static_cast<float>(state.volumePercent / 100.0f));
    }
    state.volumePercent = std::clamp(state.volumePercent, 0, 100);

    drawSoundCombo(state.currentSound, state);

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
  drawConfigCombo(state.currentConfig);

  if (ImGui::Button("Save config")) {
    Config::saveConfig(Config::configs[state.currentConfig],
                       Config::activeConfig);
  }

  ImGui::SameLine();

  if (ImGui::Button("Load config")) {
    if (Config::loadConfig(Config::configs[state.currentConfig])) {
      restoreFromConfig(state, Config::activeConfig);

      if (!Input::loadLayout(Config::activeConfig.selectedLayout)) {
        Helper::errorMesBox(L"Failed to load keyboard layout from config");
      }

      if (!Sound::loadSoundPack(Config::activeConfig.soundPack.name)) {
        Helper::errorMesBox(L"Failed to load sound pack from config");
      }

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
    handleCreateConfig(state);
    if (state.configNameError.empty()) {
      Config::refreshConfigs();
    }
  }

  if (!state.configNameError.empty()) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.2f, 0.2f, 1));
    ImGui::TextUnformatted(state.configNameError.c_str());
    ImGui::PopStyleColor();
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
  static SettingsTab currentTab = SettingsTab::Visuals;

  if (visibleNow && !wasVisibleLastFrame) {
    Input::refreshLayouts();
    Config::refreshConfigs();
    Sound::refreshSounds();

    editState.previous = Config::activeConfig;
    restoreFromConfig(editState, editState.previous);

    editState.currentConfig =
        findIndexByName(Config::configs, Config::activeConfigName);
    editState.currentLayout =
        findIndexByName(Input::layouts, Config::activeConfig.selectedLayout);
    editState.currentSound =
        findIndexByName(Sound::sounds, Config::activeConfig.soundPack.name);
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
                   ImGuiWindowFlags_NoSavedSettings);

  for (size_t i{0}; i < std::size(tabs); ++i) {
    if (ImGui::Button(tabs[i].label,
                      ImVec2((300.0f / std::size(tabs) - 9),
                             25.0f))) // todo remove magic numbers
      currentTab = tabs[i].type;

    if (i + 1 < std::size(tabs))
      ImGui::SameLine();
  }

  switch (currentTab) {
  case SettingsTab::Visuals:
    drawVisualsTab(editState);
    break;
  case SettingsTab::Sounds:
    drawSoundsTab(editState);
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

  Config::Settings newSettings = {
      Consts::CATBOARD_VERSION,
      Config::Theme{
          toConfigColor(editState.idleFillCol),
          toConfigColor(editState.activeFillCol),
          toConfigColor(editState.idleTextCol),
          toConfigColor(editState.activeTextCol),
          editState.rounding,
          editState.outline,
      },
      editState.selectedLayout,
      Config::SoundPackSelection{
          editState.selectedSoundPack.name, editState.selectedSoundPack.enable,
          editState.selectedSoundPack.layoutKeysOnlySound,
          static_cast<float>(editState.volumePercent) / 100.0f}};

  // Passing new settings but dont saving them
  Config::activeConfig = newSettings;

  if (currentTab == SettingsTab::Visuals || currentTab == SettingsTab::Sounds) {
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

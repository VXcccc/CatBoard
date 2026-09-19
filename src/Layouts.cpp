#include "Layouts.h"
#include "Config.h"
#include "Constants.h"
#include "Helpers.h"
#include "MainWindow.h"
#include "Paths.h"
#include "ConfigJson.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>
using json = nlohmann::json;

namespace Layouts {
std::vector<std::string> layouts{};
Layout activeLayout{};
std::string defaultLayoutName{"layout7t"};

void refreshLayouts() {
  Helper::refresh(layouts, Paths::layoutDirectory(), ".json");
}

bool installDefaultLayouts() {
  const auto source{Paths::getExecutableDir() / "Layouts"};
  const auto destination{Paths::layoutDirectory()};

  if (std::filesystem::exists(destination)) {
    return true;
  }

  std::error_code ec;
  std::filesystem::copy(source, destination,
                        std::filesystem::copy_options::recursive, ec);
  return !ec;
}

bool loadLayoutFile(std::string_view name) {
  try {
    std::ifstream file{Paths::getLayoutPath(name)};
    if (!file)
      return false;

    activeLayout = json::parse(file).get<Layout>();
    MainWindow::ApplyOverlaySize();
    return true;
  } catch (const std::exception &) {
    return false;
  }
}

bool loadLayout(std::string_view name) {
  if (loadLayoutFile(name))
    return true;

  if (name != defaultLayoutName)
    return loadLayoutFile(defaultLayoutName);

  return false;
}

void saveUndoClearRedo(LayoutEditState &state) {
  if (state.undoLayouts.size() >= Consts::MAX_UNDO_STEPS) {
    state.undoLayouts.erase(state.undoLayouts.begin());
  }
  state.undoLayouts.emplace_back(state.layout);
  state.redoLayouts.clear();
}

void undo(LayoutEditState &state) {
  if (state.undoLayouts.empty())
    return;
  state.redoLayouts.emplace_back(state.layout);
  state.layout = state.undoLayouts.back();
  state.undoLayouts.pop_back();
  state.selectedKeyIndex = -1;
  state.mouseCenterSelected = false;
}

void redo(LayoutEditState &state) {
  if (state.redoLayouts.empty())
    return;
  state.undoLayouts.emplace_back(state.layout);
  state.layout = state.redoLayouts.back();
  state.redoLayouts.pop_back();
  state.selectedKeyIndex = -1;
  state.mouseCenterSelected = false;
}

void deleteSelectedKey(LayoutEditState &state) {
  if (state.selectedKeyIndex < 0 ||
      state.selectedKeyIndex >= (int)state.layout.keys.size())
    return;
  state.layout.keys.erase(state.layout.keys.begin() + state.selectedKeyIndex);
  state.selectedKeyIndex = -1;
}

void handleEditorShortcuts(LayoutEditState &state) {
  if (ImGui::GetIO().WantTextInput)
    return;

  if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z)) {
    redo(state);
    return;
  }

  if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z)) {
    undo(state);
    return;
  }

  if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !state.mouseCenterSelected) {
    saveUndoClearRedo(state);
    deleteSelectedKey(state);
  }
}

void drawLayoutEditorCanvas(LayoutEditState &state) {
  handleEditorShortcuts(state);

  ImVec2 origin{ImGui::GetCursorScreenPos()};
  ImDrawList *draw{ImGui::GetWindowDrawList()};
  const auto whiteCol32 = IM_COL32(255, 255, 255, 255);

  draw->AddRect(origin,
                {origin.x + state.layout.windowWidth,
                 origin.y + state.layout.windowHeight},
                IM_COL32(255, 255, 255, 80));

  ImGui::SetCursorScreenPos(origin);
  ImGui::SetNextItemAllowOverlap();
  ImGui::InvisibleButton(
      "canvas bg", ImVec2(state.layout.windowWidth, state.layout.windowHeight));

  if (ImGui::IsItemClicked()) {
    state.selectedKeyIndex = -1;
    state.mouseCenterSelected = false;
  }

  for (int i{0}; i < static_cast<int>(state.layout.keys.size()); ++i) {
    Key &key{state.layout.keys[i]};
    ImVec2 keyPos{origin.x + key.x, origin.y + key.y};
    ImVec2 keyPosMax{keyPos.x + key.w, keyPos.y + key.h};

    ImGui::PushID(i);
    ImGui::SetCursorScreenPos(keyPos);
    ImGui::SetNextItemAllowOverlap();
    ImGui::InvisibleButton("body", ImVec2(key.w, key.h));

    if (ImGui::IsItemActivated()) {
      state.selectedKeyIndex = i;
      state.mouseCenterSelected = false;
      saveUndoClearRedo(state);
    }
    if (ImGui::IsItemActive() &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      ImVec2 delta{ImGui::GetIO().MouseDelta};
      key.x += delta.x;
      key.y += delta.y;
    }

    bool isSelected{!state.mouseCenterSelected && state.selectedKeyIndex == i};
    ImU32 fill{isSelected ? IM_COL32(80, 140, 255, 180)
                          : IM_COL32(60, 60, 60, 180)};

    draw->AddRectFilled(keyPos, keyPosMax, fill,
                        Config::activeConfig.theme.keyRounding);
    draw->AddRect(keyPos, keyPosMax, whiteCol32,
                  Config::activeConfig.theme.keyRounding);

    ImVec2 textSize{ImGui::CalcTextSize(key.label.c_str())};
    ImVec2 textPos{keyPos.x + (key.w - textSize.x) * 0.5f,
                   keyPos.y + (key.h - textSize.y) * 0.5f};
    draw->AddText(textPos, whiteCol32, key.label.c_str());

    constexpr float handleSize{5.0f};
    ImVec2 handlePos{keyPos.x + key.w - handleSize,
                     keyPos.y + key.h - handleSize};
    ImGui::SetCursorScreenPos(handlePos);
    ImGui::InvisibleButton("resize", ImVec2(handleSize, handleSize));

    if (ImGui::IsItemActivated()) {
      state.selectedKeyIndex = i;
      state.mouseCenterSelected = false;
      saveUndoClearRedo(state);
    }
    if (ImGui::IsItemActive() &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      ImVec2 delta{ImGui::GetIO().MouseDelta};
      key.w = (std::max)(10.0f, key.w + delta.x);
      key.h = (std::max)(10.0f, key.h + delta.y);
    }

    draw->AddRectFilled(handlePos,
                        {handlePos.x + handleSize, handlePos.y + handleSize},
                        IM_COL32(255, 255, 255, 255));

    ImGui::PopID();
  }

  {
    ImVec2 mousePos{origin.x + state.layout.mouse.mouseCenterX,
                    origin.y + state.layout.mouse.mouseCenterY};

    ImGui::PushID("mouse_center");
    ImGui::SetCursorScreenPos({mousePos.x - 6.0f, mousePos.y - 6.0f});
    ImGui::SetNextItemAllowOverlap();
    ImGui::InvisibleButton("mouse_center_handle", ImVec2(12.0f, 12.0f));

    if (ImGui::IsItemActivated()) {
      state.mouseCenterSelected = true;
      state.selectedKeyIndex = -1;
      saveUndoClearRedo(state);
    }
    if (ImGui::IsItemActive() &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      ImVec2 delta{ImGui::GetIO().MouseDelta};
      state.layout.mouse.mouseCenterX += delta.x;
      state.layout.mouse.mouseCenterY += delta.y;
    }

    ImU32 fill{state.mouseCenterSelected ? IM_COL32(80, 140, 255, 220)
                                         : IM_COL32(200, 80, 80, 220)};
    draw->AddCircleFilled(mousePos, 6.0f, fill);
    draw->AddCircle(mousePos, 6.0f, whiteCol32, 16, 1.5f);

    ImGui::PopID();
  }

  // debug
  char debugText[96];
  snprintf(debugText, sizeof(debugText),
           "selectedKeyIndex: %d, mouseCenterSelected: %d",
           state.selectedKeyIndex, state.mouseCenterSelected);
  draw->AddText({origin.x + 10.0f, origin.y + 10.0f},
                IM_COL32(255, 255, 0, 255), debugText);
}

void drawLayoutEditorProperties(LayoutEditState &state) {
  if (state.mouseCenterSelected) {
    ImGui::Text("Mouse center");

    ImGui::DragFloat("X", &state.layout.mouse.mouseCenterX, 1.0f);
    if (ImGui::IsItemActivated())
      saveUndoClearRedo(state);

    ImGui::DragFloat("Y", &state.layout.mouse.mouseCenterY, 1.0f);
    if (ImGui::IsItemActivated())
      saveUndoClearRedo(state);

    return;
  }

  if (state.selectedKeyIndex < 0 ||
      state.selectedKeyIndex >= (int)state.layout.keys.size()) {
    ImGui::TextDisabled("No key selected");
    return;
  }

  Key &key{state.layout.keys[state.selectedKeyIndex]};

  ImGui::InputText("##label", &key.label);
  if (ImGui::IsItemActivated())
    saveUndoClearRedo(state);

  ImGui::Text("Key code: %d", key.keyCode);
  ImGui::SameLine();
  if (ImGui::Button(state.recordingKeyCode ? "Press a key..." : "Record key")) {
    state.recordingKeyCode = true;
  }
  if (state.recordingKeyCode) {
    for (int vk{1}; vk < 256; ++vk) {
      if (GetAsyncKeyState(vk) & 0x8000) {
        key.keyCode = vk;
        state.recordingKeyCode = false;
        break;
      }
    }
    saveUndoClearRedo(state);
  }

  ImGui::DragFloat("X", &key.x, 1.0f);
  if (ImGui::IsItemActivated())
    saveUndoClearRedo(state);

  ImGui::DragFloat("Y", &key.y, 1.0f);
  if (ImGui::IsItemActivated())
    saveUndoClearRedo(state);

  ImGui::DragFloat("W", &key.w, 1.0f, 1.0f, 2000.0f);
  if (ImGui::IsItemActivated())
    saveUndoClearRedo(state);

  ImGui::DragFloat("H", &key.h, 1.0f, 1.0f, 2000.0f);
  if (ImGui::IsItemActivated())
    saveUndoClearRedo(state);

  if (ImGui::Button("Delete key")) {
    saveUndoClearRedo(state);
    deleteSelectedKey(state);
  }
}

void drawLayoutEditorTab(LayoutEditState &state) {
  if (ImGui::Button("Edit current layout")) {
    state.layout = activeLayout;
    state.selectedKeyIndex = -1;
    state.mouseCenterSelected = false;
  }

  if (ImGui::Button("Add key")) {
    std::pair<float, float> lastKeySize =
        state.layout.keys.empty()
            ? std::pair<float, float>{40.0f, 40.0f}
            : std::pair<float, float>{state.layout.keys.back().w,
                                      state.layout.keys.back().h};

    Key newKey{0, "", 10, 10, lastKeySize.first, lastKeySize.second};
    saveUndoClearRedo(state);
    state.layout.keys.emplace_back(newKey);
    state.selectedKeyIndex = (int)state.layout.keys.size() - 1;
    state.mouseCenterSelected = false;
  }

  ImGui::SetNextItemWidth(150);
  ImGui::InputText("##LayoutName", &state.layoutNameInput);
  ImGui::SameLine();
  if (ImGui::Button("Save layout")) {
    std::ofstream file{Paths::getLayoutPath(state.layoutNameInput)};
    file << json(state.layout).dump(4);
    refreshLayouts();
  }

  ImGui::SliderInt("Window width", &state.layout.windowWidth, 0, 1920);
  ImGui::SliderInt("Window height", &state.layout.windowHeight, 0, 1080);

  ImGui::Separator();

  ImGui::BeginChild(
      "canvas",
      ImVec2(state.layout.windowWidth + 20, state.layout.windowHeight + 20),
      true);
  drawLayoutEditorCanvas(state);
  ImGui::EndChild();

  ImGui::SameLine();

  ImGui::BeginChild("properties", ImVec2(250, 250), true);
  drawLayoutEditorProperties(state);
  ImGui::EndChild();
}
} // namespace Layouts

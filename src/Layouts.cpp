#include "Layouts.h"
#include "Config.h"
#include "ConfigJson.h"
#include "Constants.h"
#include "Helpers.h"
#include "MainWindow.h"
#include "Paths.h"

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

Helper::FileCreateResult saveLayout(std::string_view name,
                                    const Layout &layout) {
  if (!Paths::ensureLayoutDirectory()) {
    return Helper::FileCreateResult::DirectoryError;
  }

  if (name.empty())
    return Helper::FileCreateResult::EmptyName;
  if (name.size() > 64)
    return Helper::FileCreateResult::NameTooLong;

  const auto path{Paths::getLayoutPath(name)};

  std::ofstream file{path};
  if (!file) {
    return Helper::FileCreateResult::FileError;
  }

  file << json(layout).dump(4);
  if (!file) {
    return Helper::FileCreateResult::FileError;
  }

  return Helper::FileCreateResult::Success;
}

bool deleteLayout(std::string_view name) {
  const auto path = Paths::getLayoutPath(name);

  if (!std::filesystem::exists(path)) {
    return false;
  }

  return std::filesystem::remove(path);
}

void saveUndoClearRedo(LayoutEditState &state) {
  if (state.undoLayouts.size() >= Consts::MAX_UNDO_STEPS) {
    state.undoLayouts.erase(state.undoLayouts.begin());
  }
  if (state.undoIndexes.size() >= Consts::MAX_UNDO_STEPS) {
    state.undoIndexes.erase(state.undoIndexes.begin());
  }

  state.undoLayouts.push_back(state.layout);
  state.redoLayouts.clear();

  state.undoIndexes.push_back(state.selectedKeyIndex);
  state.redoIndexes.clear();
}

void clearUndo(LayoutEditState &state) {
  state.undoLayouts.clear();
  state.undoIndexes.clear();

  state.redoLayouts.clear();
  state.redoIndexes.clear();
}

void undo(LayoutEditState &state) {
  if (state.undoLayouts.empty())
    return;
  state.redoLayouts.push_back(state.layout);
  state.redoIndexes.push_back(state.selectedKeyIndex);

  state.layout = state.undoLayouts.back();
  state.undoLayouts.pop_back();

  if (!state.undoIndexes.empty()) {
    state.selectedKeyIndex = state.undoIndexes.back();
    state.undoIndexes.pop_back();
  }

  state.mouseCenterSelected = false;
}

void redo(LayoutEditState &state) {
  if (state.redoLayouts.empty())
    return;
  state.undoLayouts.push_back(state.layout);
  state.undoIndexes.push_back(state.selectedKeyIndex);

  state.layout = state.redoLayouts.back();
  state.redoLayouts.pop_back();

  if (!state.redoIndexes.empty()) {
    state.selectedKeyIndex = state.redoIndexes.back();
    state.redoIndexes.pop_back();
  }

  state.mouseCenterSelected = false;
}

void recordKeyCode(LayoutEditState &state, Key &key) {
  if (state.recordingKeyCode) {
    for (int vk{1}; vk < 256; ++vk) {
      if (GetAsyncKeyState(vk) & 0x8000) {
        saveUndoClearRedo(state);
        key.keyCode = vk;
        if (key.label.empty()) {
          key.label = Helper::VkToName(vk);
        }
        state.recordingKeyCode = false;
        break;
      }
    }
  }
}

void addKey(LayoutEditState &state) {
  std::pair<float, float> lastKeySize{
      state.layout.keys.empty()
          ? std::pair<float, float>{40.0f, 40.0f}
          : std::pair<float, float>{state.layout.keys.back().w,
                                    state.layout.keys.back().h}};

  std::pair<float, float> lastKeyPos{
      state.layout.keys.empty()
          ? std::pair<float, float>{10.0f, 10.0f}
          : std::pair<float, float>{state.layout.keys.back().x +
                                        lastKeySize.first + 1.0f,
                                    state.layout.keys.back().y}};

  Key newKey{0,
             "",
             lastKeyPos.first,
             lastKeyPos.second,
             lastKeySize.first,
             lastKeySize.second};
  saveUndoClearRedo(state);
  state.layout.keys.push_back(newKey);
  state.selectedKeyIndex = (int)state.layout.keys.size() - 1;
  state.mouseCenterSelected = false;

  //insta record after adding key
  state.recordingKeyCode = true;
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

  // mouse
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
  recordKeyCode(state, key);

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
    saveUndoClearRedo(state);
    state.layout = activeLayout;
    state.selectedKeyIndex = -1;
    state.mouseCenterSelected = false;
  }

  ImGui::SameLine();

  if (ImGui::Button("Reset layout")) {
    saveUndoClearRedo(state);
    state.layout.keys.clear();
    state.selectedKeyIndex = -1;
    state.mouseCenterSelected = false;
  }

  if (ImGui::Button("Add key")) {
    addKey(state);
  }

  ImGui::SameLine();

  ImGui::SetNextItemWidth(150);
  ImGui::InputText("##LayoutName", &state.layoutNameInput);
  ImGui::SameLine();
  if (ImGui::Button("Save layout")) {
    clearUndo(state);
    state.layoutNameError = Helper::getFileCreateError(
        saveLayout(state.layoutNameInput, state.layout));
    if (state.layoutNameError.empty()) {
      refreshLayouts();
    }
  }

  if (!state.layoutNameError.empty()) {
    Helper::drawErrorInSettings(state.layoutNameError);
  }

  // todo get a user resolution to clamp beetwen them
  ImGui::SliderInt("Window width", &state.layout.windowWidth, 10, 1920);
  state.layout.windowWidth = std::clamp(state.layout.windowWidth, 10, 1920);

  ImGui::SliderInt("Window height", &state.layout.windowHeight, 10, 1080);
  state.layout.windowHeight = std::clamp(state.layout.windowHeight, 10, 1080);

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

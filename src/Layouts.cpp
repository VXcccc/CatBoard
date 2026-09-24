#include "Layouts.h"
#include "Config.h"
#include "ConfigJson.h"
#include "Constants.h"
#include "Helpers.h"
#include "MainWindow.h"
#include "Paths.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>
#include <winscard.h>
#include <winuser.h>
using json = nlohmann::json;

namespace Layouts {
std::vector<std::string> layouts{};
Layout activeLayout{};
std::string defaultLayoutName{"layout7t"};

bool operator==(const Key &k1, const Key &k2) {
  return (k1.x == k2.x && k1.y == k2.y && k1.w == k2.w && k1.h == k2.h &&
          k1.label == k2.label && k1.keyCode == k2.keyCode &&
          k1.shiftLabel == k2.shiftLabel);
}
bool operator!=(const Key &k1, const Key &k2) { return !(k1 == k2); }

void refreshLayouts() {
  Helper::refresh(layouts, Paths::layoutDirectory(), ".json");
}

bool installDefaultLayouts() {
  return Helper::installDefault(Paths::getExecutableDir() / "Layouts",
                                Paths::layoutDirectory());
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
  if (state.undoSelections.size() >= Consts::MAX_UNDO_STEPS) {
    state.undoSelections.erase(state.undoSelections.begin());
  }

  state.undoLayouts.push_back(state.layout);
  state.redoLayouts.clear();

  state.undoSelections.push_back(state.selection);
  state.redoSelections.clear();
}

void clearUndo(LayoutEditState &state) {
  state.undoLayouts.clear();
  state.undoSelections.clear();

  state.redoLayouts.clear();
  state.redoSelections.clear();
}

void undo(LayoutEditState &state) {
  if (state.undoLayouts.empty())
    return;
  state.redoLayouts.push_back(state.layout);
  state.redoSelections.push_back(state.selection);

  state.layout = state.undoLayouts.back();
  state.undoLayouts.pop_back();

  if (!state.undoLayouts.empty()) {
    state.selection = state.undoSelections.back();
    state.undoSelections.pop_back();
  }

  state.mouseCenterSelected = false;
}

void redo(LayoutEditState &state) {
  if (state.redoLayouts.empty())
    return;
  state.undoLayouts.push_back(state.layout);
  state.undoSelections.push_back(state.selection);

  state.layout = state.redoLayouts.back();
  state.redoLayouts.pop_back();

  if (!state.redoSelections.empty()) {
    state.selection = state.redoSelections.back();
    state.redoSelections.pop_back();
  }

  state.mouseCenterSelected = false;
}

void setKeyProperty(Key &key, KeyProp prop, float value) {
  switch (prop) {
  case KeyProp::X:
    key.x = value;
    break;
  case KeyProp::Y:
    key.y = value;
    break;
  case KeyProp::W:
    key.w = value;
    break;
  case KeyProp::H:
    key.h = value;
    break;
  default:
    break;
  }
}

float getKeyProperty(const Key &key, KeyProp prop) {
  switch (prop) {
  case KeyProp::X:
    return key.x;
  case KeyProp::Y:
    return key.y;
  case KeyProp::W:
    return key.w;
  case KeyProp::H:
    return key.h;
  default:
    break;
  }

  return 0.0f;
}

bool isKeysPropsEqual(LayoutEditState &state, KeyProp prop) {
  if (state.selection.Size < 2)
    return true;

  void *it{nullptr};
  ImGuiID id{};

  if (!state.selection.GetNextSelectedItem(&it, &id))
    return true;

  const float firstValue{
      getKeyProperty(state.layout.keys[static_cast<int>(id)], prop)};

  while (state.selection.GetNextSelectedItem(&it, &id)) {
    const float value{
        getKeyProperty(state.layout.keys[static_cast<int>(id)], prop)};

    if (value != firstValue)
      return false;
  }

  return true;
}

void setSelectedKeysProperty(LayoutEditState &state, KeyProp prop,
                             float value) {
  void *it{nullptr};
  ImGuiID id{};

  if (!state.selection.GetNextSelectedItem(&it, &id))
    return;

  const int firstIdx{static_cast<int>(id)};
  Key &firstKey{state.layout.keys[firstIdx]};

  const bool sameY{isKeysPropsEqual(state, KeyProp::Y)};
  const bool sameX{isKeysPropsEqual(state, KeyProp::X)};

  const float oldValue{getKeyProperty(firstKey, prop)};
  const float delta{value - oldValue};

  setKeyProperty(firstKey, prop, value);

  int offset{1};

  while (state.selection.GetNextSelectedItem(&it, &id)) {
    const int idx{static_cast<int>(id)};
    Key &key{state.layout.keys[idx]};

    if (state.moveWhenMultiResize) {
      if (prop == KeyProp::W && sameY) {
        key.x += delta * offset;
      } else if (prop == KeyProp::H && sameX) {
        key.y += delta * offset;
      }
    }

    setKeyProperty(key, prop, value);

    ++offset;
  }
}

void drawMultiKeyProperty(LayoutEditState &state, KeyProp prop) {
  if (!isKeysPropsEqual(state, prop))
    return;

  void *it{nullptr};
  ImGuiID id{};

  if (!state.selection.GetNextSelectedItem(&it, &id))
    return;

  float value{getKeyProperty(state.layout.keys[static_cast<int>(id)], prop)};

  float min{0.0f};
  float max{0.0f};

  if (prop == KeyProp::W || prop == KeyProp::H) {
    min = 1.0f;
    max = 2000.0f;
  }

  const bool changed{
      ImGui::DragFloat(KeyPropName[static_cast<int>(prop)].c_str(), &value,
                       state.step, min, max)};

  if (ImGui::IsItemActivated())
    saveUndoClearRedo(state);

  if (changed)
    setSelectedKeysProperty(state, prop, value);
}

float mostFreqWH(const std::vector<Key> &keys, KeyProp prop) {

  int maxCount{0};
  float result{0.0f};

  for (const auto &key : keys) {
    const float value{getKeyProperty(key, prop)};

    int count{0};
    for (const auto &otherKey : keys) {
      if (value == getKeyProperty(otherKey, prop)) {
        ++count;
      }
    }

    if (count > maxCount || (count == maxCount && value > result)) {
      maxCount = count;
      result = value;
    }
  }

  return result;
}

void recordKeyCode(LayoutEditState &state, Key &key) {
  if (state.recordingKeyCode) {
    for (int vk{1}; vk < 256; ++vk) {
      if (vk == VK_CONTROL || vk == VK_SHIFT || vk == VK_MENU) {
        continue;
      }
      if (GetAsyncKeyState(vk) & 0x8000) {
        saveUndoClearRedo(state);
        key.keyCode = vk;
        if (key.label.empty()) {
          key.label = Helper::VkToName(vk);
        }
        state.recordingKeyCode = false;

        state.focusLabelInput = true;
        break;
      }
    }
  }
}

void addKey(LayoutEditState &state) {
  state.selection.Clear();
  std::pair<float, float> lastKeySize{state.defaultKeyWidth,
                                      state.defaultKeyHeight};

  std::pair<float, float> lastKeyPos{
      state.layout.keys.empty()
          ? std::pair<float, float>{10.0f, 10.0f}
          : std::pair<float, float>{state.layout.keys.back().x +
                                        lastKeySize.first + state.gap,
                                    state.layout.keys.back().y}};

  Key newKey{0,
             "",
             "",
             lastKeyPos.first,
             lastKeyPos.second,
             lastKeySize.first,
             lastKeySize.second};
  saveUndoClearRedo(state);
  state.layout.keys.push_back(newKey);
  state.selection.SetItemSelected((int)state.layout.keys.size() - 1, true);
  state.mouseCenterSelected = false;

  // insta record after adding key
  state.recordingKeyCode = true;
  // state.focusLabelInput = true;
}

void deleteSelectedKeys(LayoutEditState &state) {
  std::vector<int> indices;
  void *it{nullptr};
  ImGuiID id;
  while (state.selection.GetNextSelectedItem(&it, &id)) {
    indices.push_back(static_cast<int>(id));
  }

  std::sort(indices.rbegin(), indices.rend());

  for (int i : indices) {
    if (i >= 0 && i < (int)state.layout.keys.size()) {
      state.layout.keys.erase(state.layout.keys.begin() + i);
    }
  }

  state.selection.Clear();
}

void moveSelectedKeys(LayoutEditState &state, const ImVec2 &delta) {
  void *it{nullptr};
  ImGuiID id;

  while (state.selection.GetNextSelectedItem(&it, &id)) {
    int idx{static_cast<int>(id)};
    state.layout.keys[idx].x += delta.x;
    state.layout.keys[idx].y += delta.y;
  }
}

void changeSelectedLetterCase(LayoutEditState &state, bool upper) {

  if (state.selection.Size == 0)
    return;
  saveUndoClearRedo(state);

  auto lambda{upper ? [](unsigned char c) { return std::toupper(c); }
                    : [](unsigned char c) { return std::tolower(c); }};
  void *it{nullptr};
  ImGuiID id{};

  while (state.selection.GetNextSelectedItem(&it, &id)) {
    const int idx{static_cast<int>(id)};
    std::string &label{state.layout.keys[idx].label};

    std::transform(label.begin(), label.end(), label.begin(), lambda);
  }
}

void handleEditorShortcuts(LayoutEditState &state) {
  std::array<std::pair<ImGuiKey, ImVec2>, 4> moveKeys{{
      {ImGuiKey_UpArrow, {0.0f, -state.step}},
      {ImGuiKey_DownArrow, {0.0f, state.step}},
      {ImGuiKey_LeftArrow, {-state.step, 0.0f}},
      {ImGuiKey_RightArrow, {state.step, 0.0f}},
  }};

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
    deleteSelectedKeys(state);
  }

  ImVec2 delta{};
  bool startedNewMove{false};

  // arrows handle
  for (const auto &[key, movement] : moveKeys) {
    if (ImGui::IsKeyPressed(key, true)) {
      delta.x += movement.x;
      delta.y += movement.y;
    }
    if (ImGui::IsKeyPressed(key, false)) {
      startedNewMove = true;
    }
  }

  if (delta.x == 0.0f && delta.y == 0.0f)
    return;

  if (startedNewMove) {
    saveUndoClearRedo(state);
  }

  if (state.selection.Size > 0 &&
      state.selection.Size < static_cast<int>(state.layout.keys.size())) {
    moveSelectedKeys(state, delta);
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

  auto &colors = ImGui::GetStyle().Colors;

  colors[ImGuiCol_NavHighlight] = ImVec4(0, 0, 0, 0);
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
      state.selection.Clear();
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

  ImGuiMultiSelectFlags msFlags{ImGuiMultiSelectFlags_BoxSelect2d |
                                ImGuiMultiSelectFlags_NoRangeSelect |
                                ImGuiMultiSelectFlags_ClearOnEscape |
                                ImGuiMultiSelectFlags_ClearOnClickVoid};

  ImGuiMultiSelectIO *msIo{ImGui::BeginMultiSelect(
      msFlags, state.selection.Size, (int)state.layout.keys.size())};
  state.selection.ApplyRequests(msIo);

  for (int i{0}; i < static_cast<int>(state.layout.keys.size()); ++i) {
    Key &key{state.layout.keys[i]};
    ImVec2 keyPos{origin.x + key.x, origin.y + key.y};
    ImVec2 keyPosMax{keyPos.x + key.w, keyPos.y + key.h};
    bool isSelected{state.selection.Contains((ImGuiID)i)};

    ImGui::PushID(i);
    ImGui::SetCursorScreenPos(keyPos);
    ImGui::SetNextItemSelectionUserData(i);
    ImGui::Selectable("##body", isSelected, ImGuiSelectableFlags_None,
                      ImVec2(key.w, key.h));

    if (ImGui::IsItemActivated()) {
      state.mouseCenterSelected = false;
      saveUndoClearRedo(state);
    }
    if (ImGui::IsItemActive() &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

      ImVec2 delta{ImGui::GetIO().MouseDelta};

      moveSelectedKeys(state, delta);

      if (!state.selection.Contains(static_cast<ImGuiID>(i))) {
        key.x += delta.x;
        key.y += delta.y;
      }
    }

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

    ImGui::PopID();
  }

  msIo = ImGui::EndMultiSelect();
  state.selection.ApplyRequests(msIo);

  // debug
  // Helper::drawDebugText("mouseCenterSelected: %d, selectedKeyIndex: %d",
  //                       state.mouseCenterSelected, state.selectedKeyIndex);
}

void drawLayoutEditorProperties(LayoutEditState &state) {
  if (state.mouseCenterSelected) {
    ImGui::Text("Mouse center");

    ImGui::DragFloat("X", &state.layout.mouse.mouseCenterX, state.step);
    if (ImGui::IsItemActivated())
      saveUndoClearRedo(state);

    ImGui::DragFloat("Y", &state.layout.mouse.mouseCenterY, state.step);
    if (ImGui::IsItemActivated())
      saveUndoClearRedo(state);

    return;
  }

  if (state.selection.Size == 0) {
    ImGui::TextDisabled("No keys selected");
    return;
  }

  void *it{nullptr};
  ImGuiID id;

  if (!state.selection.GetNextSelectedItem(&it, &id))
    return;

  int idx{static_cast<int>(id)};

  Key &key{state.layout.keys[idx]};

  if (state.selection.Size == 1) {
    if (state.focusLabelInput) {
      ImGui::SetKeyboardFocusHere();
      state.focusLabelInput = false;
    }

    ImGui::InputText("Label", &key.label);
    if (ImGui::IsItemActivated())
      saveUndoClearRedo(state);

    ImGui::InputText("Shift label", &key.shiftLabel);
    if (ImGui::IsItemActivated())
      saveUndoClearRedo(state);

    ImGui::Text("Key code: %d", key.keyCode);
    ImGui::SameLine();
    if (ImGui::Button(state.recordingKeyCode ? "Press a key..."
                                             : "Record key")) {
      state.recordingKeyCode = true;
    }

    recordKeyCode(state, key);

  } else if (state.selection.Size > 1) {
    ImGui::Text("%d keys selected", state.selection.Size);
  }

  drawMultiKeyProperty(state, KeyProp::X);
  drawMultiKeyProperty(state, KeyProp::Y);
  drawMultiKeyProperty(state, KeyProp::W);
  drawMultiKeyProperty(state, KeyProp::H);

  if (ImGui::Button("UPPER")) {
    changeSelectedLetterCase(state, true);
  }

  ImGui::SameLine();

  if (ImGui::Button("lower")) {
    changeSelectedLetterCase(state, false);
  }

  std::string deleteButtonLabel{state.selection.Size > 1 ? "Delete keys"
                                                         : "Delete key"};
  if (ImGui::Button(deleteButtonLabel.c_str())) {
    saveUndoClearRedo(state);
    deleteSelectedKeys(state);
  }
}

void drawLayoutEditorOptions(LayoutEditState &state) {
  ImGui::BeginChild("Options", ImVec2(410, 120), true);

  if (ImGui::Button("Edit current layout")) {
    saveUndoClearRedo(state);
    state.layout = activeLayout;
    state.selection.Clear();
    state.mouseCenterSelected = false;
    state.layoutNameInput = Config::activeConfig.selectedLayout;
    state.defaultKeyHeight = mostFreqWH(state.layout.keys, KeyProp::H);
    state.defaultKeyWidth = mostFreqWH(state.layout.keys, KeyProp::W);
  }

  ImGui::SameLine();

  if (ImGui::Button("Reset layout")) {
    saveUndoClearRedo(state);
    state.layout.keys.clear();
    state.selection.Clear();
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
    const auto path = Paths::getLayoutPath(state.layoutNameInput);

    if (std::filesystem::exists(path)) {
      ImGui::OpenPopup("Save Layout");
    } else {
      clearUndo(state);

      state.layoutNameError = Helper::getFileCreateError(
          saveLayout(state.layoutNameInput, state.layout));

      if (state.layoutNameError.empty()) {
        refreshLayouts();
      }
    }
  }

  if (ImGui::BeginPopupModal("Save Layout", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {

    ImGui::Text("A layout with this name already exists.\n"
                "Do you want to overwrite it?");

    if (ImGui::Button("Yes", ImVec2(100, 0))) {
      clearUndo(state);

      state.layoutNameError = Helper::getFileCreateError(
          saveLayout(state.layoutNameInput, state.layout));

      if (state.layoutNameError.empty()) {
        refreshLayouts();

        if (state.layoutNameInput == Config::activeConfig.selectedLayout) {
          loadLayout(state.layoutNameInput);
        }
      }

      ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("No", ImVec2(100, 0))) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  if (!state.layoutNameError.empty()) {
    Helper::drawErrorInSettings(state.layoutNameError);
  }

  // todo get a user resolution to clamp beetwen them
  ImGui::SetNextItemWidth(300);
  ImGui::SliderInt("Window width", &state.layout.windowWidth, 10, 1920);
  state.layout.windowWidth = std::clamp(state.layout.windowWidth, 10, 1920);

  ImGui::SetNextItemWidth(300);
  ImGui::SliderInt("Window height", &state.layout.windowHeight, 10, 1080);
  state.layout.windowHeight = std::clamp(state.layout.windowHeight, 10, 1080);

  ImGui::EndChild();

  ImGui::SameLine();

  ImGui::BeginChild("add key options", ImVec2(410, 120), true);

  ImGui::SetNextItemWidth(300);
  ImGui::SliderFloat("Gap", &state.gap, 0.0f, 10.0f, "%.2f");
  state.gap = std::clamp(state.gap, -20.0f, 100.0f); // freedom

  ImGui::SetNextItemWidth(300);
  ImGui::SliderFloat("Step", &state.step, 0.0f, 10.0f, "%.2f");
  state.step = std::clamp(state.step, 0.0f, 100.0f);

  ImGui::SetNextItemWidth(300);
  ImGui::SliderFloat("Key width", &state.defaultKeyWidth, 10.0f, 200.0f,
                     "%.0f");
  state.defaultKeyWidth = std::clamp(state.defaultKeyWidth, 10.0f, 200.0f);

  ImGui::SetNextItemWidth(300);
  ImGui::SliderFloat("Key height", &state.defaultKeyHeight, 10.0f, 200.0f,
                     "%.0f");
  state.defaultKeyHeight = std::clamp(state.defaultKeyHeight, 10.0f, 200.0f);

  ImGui::Checkbox("Move with multi resize", &state.moveWhenMultiResize);

  ImGui::EndChild();
}

void drawLayoutEditorTab(LayoutEditState &state) {
  ImDrawList *draw{ImGui::GetWindowDrawList()};

  drawLayoutEditorOptions(state);

  ImGui::SeparatorText("");

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

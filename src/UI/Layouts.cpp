#include "Layouts.h"
#include "AppState.h"
#include "Config.h"
#include "Constants.h"
#include "Fonts.h"
#include "Helpers.h"
#include "JsonParser.h"
#include "Keyboard.h"
#include "MainWindow.h"
#include "Mouse.h"
#include "Paths.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <synchapi.h>
#include <vector>
#include <winscard.h>
#include <winuser.h>
using json = nlohmann::json;

#include <iostream>

namespace Layouts {
std::vector<std::string> layouts{};
const std::string defaultLayoutName{"layout7t"};
const Layout defaultLayout{makeDefaultLayout()};

// float epsilon
static constexpr float s_eps{0.001f};

inline const std::array<Key, 25> defaultKeysArr{
    Key{KeyType::Keyboard, 9, "Tab", "", 9.0f, 53.0f, 67.0f, 43.0f},
    Key{KeyType::Keyboard, 49, "1", "!", 53.0f, 9.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 50, "2", "@", 97.0f, 9.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 51, "3", "#", 141.0f, 9.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 52, "4", "$", 185.0f, 9.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 53, "5", "%", 229.0f, 9.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 81, "q", "", 77.0f, 53.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 87, "w", "", 121.0f, 53.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 69, "e", "", 165.0f, 53.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 82, "r", "", 209.0f, 53.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 84, "t", "", 253.0f, 53.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 20, "Caps", "", 9.0f, 97.0f, 79.0f, 43.0f},
    Key{KeyType::Keyboard, 65, "a", "", 89.0f, 97.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 83, "s", "", 133.0f, 97.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 68, "d", "", 177.0f, 97.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 70, "f", "", 221.0f, 97.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 71, "g", "", 265.0f, 97.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 160, "Shift", "", 9.0f, 141.0f, 100.0f, 43.0f},
    Key{KeyType::Keyboard, 90, "z", "", 110.0f, 141.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 88, "x", "", 154.0f, 141.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 67, "c", "", 198.0f, 141.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 86, "v", "", 242.0f, 141.0f, 43.0f, 43.0f},
    Key{KeyType::Keyboard, 162, "Ctrl", "", 9.0f, 185.0f, 67.0f, 43.0f},
    Key{KeyType::Keyboard, 164, "Alt", "", 119.0f, 185.0f, 67.0f, 43.0f},
    Key{KeyType::Keyboard, 32, "", "", 187.0f, 185.0f, 121.0f, 43.0f},
};

Layout makeDefaultLayout() {
  Layout layout{450, 287};
  layout.mouse = {374.0f, 184.0f, 100.0f, 75.0f, 37.0f};
  layout.keys.reserve(defaultKeysArr.size());
  for (const auto &Key : defaultKeysArr) {
    layout.keys.push_back(Key);
  }
  return layout;
}

void refreshLayouts() {
  Helper::refresh(layouts, Paths::layoutDirectory(), ".json");
}

bool loadLayoutFile(std::string_view name, AppState &app) {
  try {
    std::ifstream file{Paths::getLayoutPath(name)};
    if (!file)
      return false;

    app.layout = json::parse(file).get<Layout>();
    MainWindow::ApplyOverlaySize(app);
    return true;
  } catch (const std::exception &) {
    return false;
  }
}

bool loadLayout(std::string_view name, AppState &app) {
  bool loaded{false};
  if (!loadLayoutFile(name, app)) {
    app.layout = defaultLayout;
    loaded = false;
  } else {
    loaded = true;
  }

  app.keyboard.resyncVisuals(app.layout, app.config.theme);

  return loaded;
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

static void sanitizeSelection(LayoutEditState &state) {
  std::vector<ImGuiID> valid;
  void *it{nullptr};
  ImGuiID id{};
  const ImGuiID idCount{static_cast<ImGuiID>(state.layout.keys.size())};
  while (state.selection.GetNextSelectedItem(&it, &id))
    if (id < idCount)
      valid.push_back(id);

  state.selection.Clear();
  for (ImGuiID v : valid)
    state.selection.SetItemSelected(v, true);
}

void undo(LayoutEditState &state) {
  if (state.undoLayouts.empty())
    return;

  state.redoLayouts.push_back(state.layout);
  state.redoSelections.push_back(state.selection);

  state.layout = state.undoLayouts.back();
  state.undoLayouts.pop_back();

  if (!state.undoSelections.empty()) {
    state.selection = state.undoSelections.back();
    state.undoSelections.pop_back();
  }

  sanitizeSelection(state);
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

  sanitizeSelection(state);
  state.mouseCenterSelected = false;
}

static void copySelected(LayoutEditState &state) {
  state.copyBuffer.clear();

  void *it = nullptr;
  ImGuiID id;
  while (state.selection.GetNextSelectedItem(&it, &id)) {
    if (id < state.layout.keys.size())
      state.copyBuffer.push_back(state.layout.keys[id]);
  }
}

static void pasteBuffer(LayoutEditState &state) {
  state.selection.Clear();
  for (const auto &key : state.copyBuffer) {
    state.layout.keys.push_back(key);
    state.selection.SetItemSelected(state.layout.keys.size() - 1, true);
  }
}

// debug
static void printKeysMatrix(std::vector<Key *> &vec, std::string_view str) {
  std::cout << str;
  for (const auto &key : vec) {
    std::cout << key->label << ", ";
  }
  std::cout << '\n';
}

using KeysMatrix = std::vector<std::vector<Key *>>;
static KeysMatrix buildRows(std::vector<Key *> keys) {
  KeysMatrix rows{};
  if (keys.empty())
    return rows;

  std::sort(keys.begin(), keys.end(), [](const Key *a, const Key *b) {
    if (std::abs(a->y - b->y) > s_eps)
      return a->y < b->y;
    return a->x < b->x;
  });

  int startIdx{0};
  for (std::size_t i{1}; i < keys.size(); ++i) {
    if (std::abs(keys[i - 1]->y - keys[i]->y) > s_eps) {
      rows.emplace_back(keys.begin() + startIdx, keys.begin() + i);
      startIdx = i;
    }
  }
  rows.emplace_back(keys.begin() + startIdx, keys.end());

  return rows;
}

static KeysMatrix getSelectedKeysMatrix(LayoutEditState &state) {
  // constexpr float eps = 0.01f;
  std::vector<Key *> keys;
  void *it = nullptr;
  ImGuiID id;
  while (state.selection.GetNextSelectedItem(&it, &id)) {
    if (id < state.layout.keys.size())
      keys.push_back(&state.layout.keys[id]);
  }

  return buildRows(std::move(keys));
}

struct RowSpan {
  float top;
  float bottom;
};

static RowSpan rowSpan(const std::vector<Key *> &row) {
  RowSpan rs{row.front()->y, (row.front()->y + row.front()->h)};
  for (const Key *k : row) {
    rs.top = (std::min)(rs.top, k->y);
    rs.bottom = (std::max)(rs.bottom, k->y + k->h);
  }
  return rs;
}

// returning nullopt if gaps are not eqaul or every row < 2
std::optional<float> getGap(const KeysMatrix &rows, KeyProp prop) {
  std::optional<float> common;

  const auto isDiffer = [&](float gap) {
    if (!common) {
      common = gap;
      return false;
    }
    return std::abs(gap - *common) > s_eps;
  };

  if (prop == KeyProp::gapX) {
    for (const auto &row : rows) {
      for (std::size_t i{0}; i + 1 < row.size(); ++i) {
        if (isDiffer(row[i + 1]->x - (row[i]->x + row[i]->w)))
          return std::nullopt;
      }
    }
  } else {
    for (std::size_t j{0}; j + 1 < rows.size(); ++j) {
      if (isDiffer(rowSpan(rows[j + 1]).top - rowSpan(rows[j]).bottom))
        return std::nullopt;
    }
  }
  return common;
}

void setGap(LayoutEditState &state, float gap, KeyProp prop) {
  const auto rows{getSelectedKeysMatrix(state)};

  if (prop == KeyProp::gapX) {
    for (const auto &row : rows) {
      for (std::size_t i{1}; i < row.size(); ++i) {
        row[i]->x = (row[i - 1]->x + row[i - 1]->w + gap);
      }
    }
    return;
  }

  if (rows.size() < 2)
    return;

  float prevBottom{rowSpan(rows[0]).bottom};
  for (std::size_t j{1}; j < rows.size(); ++j) {
    const RowSpan rs{rowSpan(rows[j])};
    const float delta{(prevBottom + gap) - rs.top};
    for (Key *k : rows[j]) {
      k->y += delta;
    }
    prevBottom = rs.bottom + delta;
  }
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

// for gaps use getGap
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
  if (state.selection.Size == 0)
    return;

  if (prop == KeyProp::gapX || prop == KeyProp::gapY) {
    setGap(state, value, prop);
    return;
  }

  const bool keepGap{state.applyGapWithMultiResize && state.selection.Size > 1};

  std::optional<float> gapX, gapY;
  if (keepGap) {
    gapX = getGap(getSelectedKeysMatrix(state), KeyProp::gapX);
    gapY = getGap(getSelectedKeysMatrix(state), KeyProp::gapY);
  }

  void *it{nullptr};
  ImGuiID id{};

  while (state.selection.GetNextSelectedItem(&it, &id)) {
    if (id < state.layout.keys.size())
      setKeyProperty(state.layout.keys[id], prop, value);
  }

  if (keepGap) {
    if (prop == KeyProp::W)
      setGap(state, gapX.value_or(state.gap), KeyProp::gapX);
    else if (prop == KeyProp::H)
      setGap(state, gapY.value_or(state.gap), KeyProp::gapY);
  }
}

void drawMultiKeyProperty(LayoutEditState &state, KeyProp prop,
                          float sliderWidth = 150.0f) {
  const bool isGap{prop == KeyProp::gapY || prop == KeyProp::gapX};
  const bool isSize{prop == KeyProp::W || prop == KeyProp::H};

  if (isGap && state.selection.Size < 2)
    return;
  if (!isGap && !isSize && !isKeysPropsEqual(state, prop))
    return;
  if (prop == KeyProp::gapY && getSelectedKeysMatrix(state).size() < 2)
    return;

  void *it{nullptr};
  ImGuiID id{};

  if (!state.selection.GetNextSelectedItem(&it, &id))
    return;

  float value{};
  bool displayGapHelper{false};
  bool displaySizeHelper{false};
  if (isGap) {
    const auto gap{getGap(getSelectedKeysMatrix(state), prop)};
    if (!gap) {
      displayGapHelper = true;
    }
    value = gap.value_or(0.0f);
  } else {
    if (id >= state.layout.keys.size())
      return;

    if (isKeysPropsEqual(state, prop))
      value = getKeyProperty(state.layout.keys[static_cast<int>(id)], prop);
    else {
      value = 0.0f;
      displaySizeHelper = true;
    }
  }

  float min{0.0f};
  float max{0.0f};

  if (prop == KeyProp::W || prop == KeyProp::H) {
    min = 1.0f;
    max = 2000.0f;
  }

  ImGui::SetNextItemWidth(sliderWidth);

  const char *label{KeyPropName[static_cast<int>(prop)].c_str()};
  const bool changed{ImGui::DragFloat(label, &value, state.step, min, max)};

  if (ImGui::IsItemActivated())
    saveUndoClearRedo(state);

  if (displayGapHelper) {
    ImGui::SameLine();
    Helper::HelpMarker("Gap diff, so the value is set to 0.0f");
  }

  if (displaySizeHelper) {
    ImGui::SameLine();
    Helper::HelpMarker("Size diff, so the value is set to 0.0f");
  }

  if (changed)
    setSelectedKeysProperty(state, prop, value);
}

static float getMostFreqGapX(LayoutEditState &state) {
  std::vector<Key *> keys;
  keys.reserve(state.layout.keys.size());
  for (Key &key : state.layout.keys)
    keys.push_back(&key);

  const KeysMatrix rows{buildRows(std::move(keys))};

  std::vector<float> gaps;
  for (const auto &row : rows) {
    for (std::size_t i{0}; i + 1 < row.size(); ++i)
      gaps.push_back(row[i + 1]->x - (row[i]->x + row[i]->w));
  }

  if (gaps.empty())
    return state.gap;

  int maxCount{0};
  float result{state.gap};

  for (const float value : gaps) {
    const int count{
        static_cast<int>(std::count(gaps.begin(), gaps.end(), value))};

    if (count > maxCount) {
      maxCount = count;
      result = value;
    }
  }

  return result;
}

static float mostFreqWH(LayoutEditState &state, KeyProp prop) {
  const std::vector<Key> &keys{state.layout.keys};
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

static float snapValue(float v, float gridSize) {
  return (std::round(v / gridSize) * gridSize);
}

static bool isKeyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

void recordKeyCode(LayoutEditState &state, Key &key,
                   const Keyboard::State &kb) {
  static Keyboard::KeyBitSet wasDown{};
  static bool snapshotTaken{false};

  if (!state.recordingKeyCode) {
    snapshotTaken = false;
    return;
  }

  if (!snapshotTaken) {
    wasDown = kb.down;
    snapshotTaken = true;
  }

  if (state.recordingKeyCode) {
    for (int vk{1}; vk < 256; ++vk) {
      if (vk == VK_CONTROL || vk == VK_SHIFT || vk == VK_MENU) {
        continue;
      }
      const bool down{kb.down[vk]};
      if (wasDown[vk]) {
        if (!down) {
          wasDown[vk] = false;
        }
        continue;
      }

      if (down) {
        saveUndoClearRedo(state);
        key.code = vk;
        state.recordingKeyCode = false;
        snapshotTaken = false;

        if (state.focusLabelAfterRecord)
          state.focusLabelInput = true;

        break;
      }
    }
  }
}

static std::pair<float, float>
getLastKeyPos(LayoutEditState &state,
              const std::pair<float, float> defaultKeySize) {
  return state.layout.keys.empty()
             ? std::pair<float, float>{10.0f, 10.0f}
             : std::pair<float, float>{state.layout.keys.back().x +
                                           defaultKeySize.first + state.gap,
                                       state.layout.keys.back().y};
}

void addKey(LayoutEditState &state, KeyType type = KeyType::Keyboard) {
  state.selection.Clear();
  std::pair<float, float> defaultKeySize{state.defaultKeyWidth,
                                         state.defaultKeyHeight};

  std::pair<float, float> lastKeyPos{getLastKeyPos(state, defaultKeySize)};

  Key newKey{type,
             0,
             "",
             "",
             lastKeyPos.first,
             lastKeyPos.second,
             defaultKeySize.first,
             defaultKeySize.second};
  saveUndoClearRedo(state);
  state.layout.keys.push_back(newKey);
  state.selection.SetItemSelected((int)state.layout.keys.size() - 1, true);
  state.mouseCenterSelected = false;

  // insta record after adding key
  if (type == KeyType::Keyboard && state.recordAfterAdding)
    state.recordingKeyCode = true;
}

void addSpecialKeyPopup(LayoutEditState &state) {
  if (ImGui::BeginPopup("SpecialKeyPopup")) {
    for (std::size_t i{0}; i < sKeysInfo.size(); ++i) {
      const auto &sKey{sKeysInfo[i]};
      if (ImGui::Selectable(sKey.name.c_str()))
        addKey(state, sKey.type);
    }
    ImGui::EndPopup();
  }
  return;
}

static void deleteSelectedKeys(LayoutEditState &state) {
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

static void deleteLabels(LayoutEditState &state) {
  void *it{nullptr};
  ImGuiID id{};
  while (state.selection.GetNextSelectedItem(&it, &id)) {
    int i{static_cast<int>(id)};
    if (i >= 0 && i < (int)state.layout.keys.size()) {
      Key &key{state.layout.keys[static_cast<int>(id)]};
      key.label = "";
      key.shiftLabel = "";
    }
  }
}

static void applySuggestedLabels(LayoutEditState &state) {
  void *it{nullptr};
  ImGuiID id{};
  while (state.selection.GetNextSelectedItem(&it, &id)) {
    int i{static_cast<int>(id)};
    if (i >= 0 && i < (int)state.layout.keys.size()) {
      Key &key{state.layout.keys[static_cast<int>(id)]};
      std::string_view suggLabel{Helper::VkToName(key.code)};
      if (suggLabel != "Unknown")
        key.label = suggLabel;

      std::string_view shiftLabel{Helper::VkToShiftLabel(key.code)};
      if (!shiftLabel.empty()) {
        key.shiftLabel = shiftLabel;
      }
    }
  }
}

static void moveSelectedKeys(LayoutEditState &state, const ImVec2 &delta) {
  void *it{nullptr};
  ImGuiID id{};
  while (state.selection.GetNextSelectedItem(&it, &id)) {
    int i{static_cast<int>(id)};
    if (i >= 0 && i < (int)state.layout.keys.size()) {
      Key &key{state.layout.keys[i]};
      key.x += delta.x;
      key.y += delta.y;
    }
  }
}

static ImVec2 computeDragDelta(
    LayoutEditState &state,
    const ImVec2 &mouseDeltaLogical) { // logical = already divided by zoom
  state.moveRaw.x += mouseDeltaLogical.x;
  state.moveRaw.y += mouseDeltaLogical.y;

  const bool snap{state.snapToGrid && !ImGui::GetIO().KeyAlt};
  const ImVec2 target{
      snap ? snapValue(state.moveRaw.x, state.gridSize) : state.moveRaw.x,
      snap ? snapValue(state.moveRaw.y, state.gridSize) : state.moveRaw.y};

  const ImVec2 delta{target.x - state.moveAnchor.x,
                     target.y - state.moveAnchor.y};
  state.moveAnchor = target;
  return delta;
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
  const bool snap{state.snapToGrid && !ImGui::GetIO().KeyAlt &&
                  state.gridSize > 0.0f};
  const float stepSize{snap ? state.gridSize : state.step};
  const std::array<std::pair<ImGuiKey, ImVec2>, 4> moveKeys{{
      {ImGuiKey_UpArrow, {0.0f, -stepSize}},
      {ImGuiKey_DownArrow, {0.0f, stepSize}},
      {ImGuiKey_LeftArrow, {-stepSize, 0.0f}},
      {ImGuiKey_RightArrow, {stepSize, 0.0f}},
  }};

  if (state.selection.Size == 1 && !state.recordingKeyCode &&
      ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_R)) {
    state.recordingKeyCode = true;
    return;
  }

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

  if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_C)) {
    copySelected(state);
    return;
  }

  if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_V)) {
    pasteBuffer(state);
    return;
  }

  if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !state.mouseCenterSelected) {
    saveUndoClearRedo(state);
    deleteSelectedKeys(state);
  }

  // scroll zoom
  const auto &io{ImGui::GetIO()};
  if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f &&
      ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsAnyItemActive() == false) {

    state.zoom = std::clamp(state.zoom * (1.0f + io.MouseWheel * 0.10f),
                            Consts::EDITOR_MINZOOM, Consts::EDITOR_MAXZOOM);
  }

  // mmb move
  if (ImGui::IsWindowHovered() &&
      ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
    state.ViewOffset.x += io.MouseDelta.x / state.zoom;
    state.ViewOffset.y += io.MouseDelta.y / state.zoom;
  }

  // arrows handle
  ImVec2 delta{};
  bool startedNewMove{false};
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
  if (state.mouseCenterSelected) {
    state.layout.mouse.centerX += delta.x;
    state.layout.mouse.centerY += delta.y;
  }
  if (state.selection.Size > 0) {
    moveSelectedKeys(state, delta);
  }
}

static void drawGrid(LayoutEditState &state, ImDrawList *draw,
                     const ImVec2 &origin) {
  if (state.gridEnabled) {
    const float step{static_cast<float>(state.gridSize) * state.zoom};
    const ImU32 gridCol{IM_COL32(255, 255, 255, 30)};

    const float w{static_cast<float>(state.layout.windowWidth * state.zoom)};
    const float h{static_cast<float>(state.layout.windowHeight * state.zoom)};

    if (step >= 4.0f) {
      for (float x{step}; x < w; x += step) {
        draw->AddLine({origin.x + x, origin.y}, {origin.x + x, origin.y + h},
                      gridCol, 1.0f);
      }
      for (float y{step}; y < h; y += step) {
        draw->AddLine({origin.x, origin.y + y}, {origin.x + w, origin.y + y},
                      gridCol, 1.0f);
      }
    }
  }
}

enum class Edge { Top, Bottom, Left, Right };

static void beginResize(LayoutEditState &state, const Key &key, Edge edge) {
  switch (edge) {
  case Edge::Top:
    state.resizeRaw = key.y;
    state.resizeFixed = key.y + key.h;
    break;
  case Edge::Bottom:
    state.resizeRaw = key.y + key.h;
    state.resizeFixed = key.y;
    break;
  case Edge::Left:
    state.resizeRaw = key.x;
    state.resizeFixed = key.x + key.w;
    break;
  case Edge::Right:
    state.resizeRaw = key.x + key.w;
    state.resizeFixed = key.x;
    break;
  }
}

static void beginMouseResize(LayoutEditState &state, float center, float size,
                             Edge edge) {
  const bool movesMinSide{edge == Edge::Top || edge == Edge::Left};

  if (movesMinSide) {
    state.resizeRaw = center - size * 0.5f;
    state.resizeFixed = center + size * 0.5f;
  } else {
    state.resizeRaw = center + size * 0.5f;
    state.resizeFixed = center - size * 0.5f;
  }
}

static void applyResize(LayoutEditState &state, Key &key, Edge edge,
                        const ImVec2 &deltaPx, float zoom) {
  const bool vertical{edge == Edge::Top || edge == Edge::Bottom};
  const bool movesMinSide{edge == Edge::Top || edge == Edge::Left};

  const bool snap{state.snapToGrid && !ImGui::GetIO().KeyAlt};
  const float minSize{snap ? state.gridSize : state.step};

  state.resizeRaw += (vertical ? deltaPx.y : deltaPx.x) / zoom;

  float pos{snap ? snapValue(state.resizeRaw, state.gridSize)
                 : state.resizeRaw};
  const float fixed{state.resizeFixed};

  if (movesMinSide) {
    pos = (std::min)(pos, fixed - minSize);
    if (vertical) {
      key.y = pos;
      key.h = fixed - pos;
    } else {
      key.x = pos;
      key.w = fixed - pos;
    }
  } else {
    pos = (std::max)(pos, fixed + minSize);
    if (vertical)
      key.h = pos - fixed;
    else
      key.w = pos - fixed;
  }
}

static void applyMouseResize(LayoutEditState &state, Edge edge, float &center,
                             float &size) {
  const bool movesMinSide{edge == Edge::Top || edge == Edge::Left};

  const bool snap{state.snapToGrid && !ImGui::GetIO().KeyAlt};
  const float minSize{snap ? state.gridSize : state.step};

  state.resizeRaw += ((edge == Edge::Top || edge == Edge::Bottom)
                          ? ImGui::GetIO().MouseDelta.y
                          : ImGui::GetIO().MouseDelta.x) /
                     state.zoom;

  float pos{snap ? snapValue(state.resizeRaw, state.gridSize)
                 : state.resizeRaw};

  const float fixed{state.resizeFixed};

  if (movesMinSide) {
    pos = (std::min)(pos, fixed - minSize);

    size = fixed - pos;
    center = (fixed + pos) * 0.5f;
  } else {
    pos = (std::max)(pos, fixed + minSize);

    size = pos - fixed;
    center = (fixed + pos) * 0.5f;
  }
}

static bool resizeHandle(LayoutEditState &state, ImDrawList *draw,
                         const char *id, Key &key, Edge edge, const ImVec2 &a,
                         const ImVec2 &b) {
  ImGui::SetCursorScreenPos(a);
  ImGui::InvisibleButton(id, {b.x - a.x, b.y - a.y});

  const bool hovered{ImGui::IsItemHovered()};
  const bool active{ImGui::IsItemActive()};

  if (ImGui::IsItemActivated()) {
    saveUndoClearRedo(state);
    beginResize(state, key, edge);
  }

  if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
    applyResize(state, key, edge, ImGui::GetIO().MouseDelta, state.zoom);

  if (hovered || active)
    ImGui::SetMouseCursor((edge == Edge::Top || edge == Edge::Bottom)
                              ? ImGuiMouseCursor_ResizeNS
                              : ImGuiMouseCursor_ResizeEW);

  draw->AddRectFilled(a, b, IM_COL32(255, 255, 255, 180));
  return active;
}

static bool mouseRectResizeHandle(LayoutEditState &state, ImDrawList *draw,
                                  const char *id, float &size, float &center,
                                  bool vertical, Edge edge, const ImVec2 &a,
                                  const ImVec2 &b) {
  ImGui::SetCursorScreenPos(a);
  ImGui::InvisibleButton(id, {b.x - a.x, b.y - a.y});

  const bool hovered{ImGui::IsItemHovered()};
  const bool active{ImGui::IsItemActive()};

  if (ImGui::IsItemActivated()) {
    saveUndoClearRedo(state);
    beginMouseResize(state, center, size, edge);
  }

  if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
    applyMouseResize(state, edge, center, size);
  }

  if (hovered || active) {
    ImGui::SetMouseCursor(vertical ? ImGuiMouseCursor_ResizeNS
                                   : ImGuiMouseCursor_ResizeEW);
  }

  draw->AddRectFilled(a, b, IM_COL32(255, 255, 255, 180));
  return active;
}

static void drawMouseRectResizeHandles(LayoutEditState &state, ImDrawList *draw,
                                       const ImVec2 &center) {
  auto &mouse{state.layout.mouse};

  const float halfW{mouse.width * 0.5f * state.zoom};
  const float halfH{mouse.height * 0.5f * state.zoom};

  constexpr float thick{6.0f};

  const float lenH{(std::min)(24.0f, halfW)};
  const float lenV{(std::min)(24.0f, halfH)};

  ImGui::PushID("mouse_rect_handles");

  mouseRectResizeHandle(state, draw, "left", mouse.width, mouse.centerX, false,
                        Edge::Left,
                        {center.x - halfW - thick * 0.5f, center.y - lenV},
                        {center.x - halfW + thick * 0.5f, center.y + lenV});

  mouseRectResizeHandle(state, draw, "right", mouse.width, mouse.centerX, false,
                        Edge::Right,
                        {center.x + halfW - thick * 0.5f, center.y - lenV},
                        {center.x + halfW + thick * 0.5f, center.y + lenV});

  mouseRectResizeHandle(state, draw, "top", mouse.height, mouse.centerY, true,
                        Edge::Top,
                        {center.x - lenH, center.y - halfH - thick * 0.5f},
                        {center.x + lenH, center.y - halfH + thick * 0.5f});

  mouseRectResizeHandle(state, draw, "bottom", mouse.height, mouse.centerY,
                        true, Edge::Bottom,
                        {center.x - lenH, center.y + halfH - thick * 0.5f},
                        {center.x + lenH, center.y + halfH + thick * 0.5f});

  ImGui::PopID();
}

static void drawResizeHandles(LayoutEditState &state, ImDrawList *draw,
                              Key &key, const ImVec2 &min, const ImVec2 &max) {
  const float w{max.x - min.x};
  const float h{max.y - min.y};

  const float thick{std::clamp((std::min)(w, h) * 0.12f, 3.0f, 6.0f)};
  constexpr float padding{6.0f};

  const float lenH{
      std::clamp(w * 0.5f, 0.0f, (std::max)(0.0f, w - 2 * padding))};
  const float lenV{
      std::clamp(h * 0.5f, 0.0f, (std::max)(0.0f, h - 2 * padding))};

  const float cx{(min.x + max.x) * 0.5f};
  const float cy{(min.y + max.y) * 0.5f};

  ImGui::PushID("resize");

  if (lenH >= 4.0f) {
    resizeHandle(state, draw, "top", key, Edge::Top, {cx - lenH * 0.5f, min.y},
                 {cx + lenH * 0.5f, min.y + thick});
    resizeHandle(state, draw, "bottom", key, Edge::Bottom,
                 {cx - lenH * 0.5f, max.y - thick}, {cx + lenH * 0.5f, max.y});
  }
  if (lenV >= 4.0f) {
    resizeHandle(state, draw, "left", key, Edge::Left,
                 {min.x, cy - lenV * 0.5f}, {min.x + thick, cy + lenV * 0.5f});
    resizeHandle(state, draw, "right", key, Edge::Right,
                 {max.x - thick, cy - lenV * 0.5f}, {max.x, cy + lenV * 0.5f});
  }

  ImGui::PopID();
}

static void drawLayoutEditorCanvas(LayoutEditState &state,
                                   const AppState &app) {
  handleEditorShortcuts(state);
  ImGui::PushStyleColor(ImGuiCol_NavHighlight, ImVec4(0, 0, 0, 0));
  const float zoom{state.zoom};

  // i ommited overlay font because it was a several glitches with Font Atlas
  // that break editor canvas
  ImGui::PushFont(Fonts::settingsFont, Fonts::settingsFontSizePixels * zoom);

  const ImVec2 cursor{ImGui::GetCursorScreenPos()};
  const ImVec2 origin{cursor.x + state.ViewOffset.x * state.zoom,
                      cursor.y + state.ViewOffset.y * state.zoom};

  ImDrawList *draw{ImGui::GetWindowDrawList()};
  const auto whiteCol32 = IM_COL32(255, 255, 255, 255);

  draw->AddRect(origin,
                {origin.x + state.layout.windowWidth * zoom,
                 origin.y + state.layout.windowHeight * zoom},
                IM_COL32(255, 255, 255, 80));

  drawGrid(state, draw, origin);

  // mouse center
  {
    const auto &mouse{app.config.theme.mouse};

    const ImVec2 mouseCenter{origin.x + state.layout.mouse.centerX * zoom,
                             origin.y + state.layout.mouse.centerY * zoom};

    ImGui::PushID("mouse_center");
    ImGui::SetCursorScreenPos({mouseCenter.x - 6.0f, mouseCenter.y - 6.0f});
    ImGui::SetNextItemAllowOverlap();
    ImGui::InvisibleButton("mouse_center_handle", ImVec2(12.0f, 12.0f));

    if (ImGui::IsItemActivated()) {
      state.mouseCenterSelected = true;
      state.selection.Clear();
      state.moveRaw = {state.layout.mouse.centerX, state.layout.mouse.centerY};
      state.moveAnchor = state.moveRaw;
      saveUndoClearRedo(state);
    }
    if (ImGui::IsItemActive() &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      const ImVec2 mouseDelta{ImGui::GetIO().MouseDelta.x / zoom,
                              ImGui::GetIO().MouseDelta.y / zoom};
      const ImVec2 delta{computeDragDelta(state, mouseDelta)};
      state.layout.mouse.centerX += delta.x;
      state.layout.mouse.centerY += delta.y;
    }
    const ImU32 fill{state.mouseCenterSelected ? IM_COL32(80, 140, 255, 220)
                                               : IM_COL32(60, 60, 60, 220)};

    draw->AddCircleFilled(mouseCenter, 6.0f, fill);
    draw->AddCircle(mouseCenter, 6.0f, whiteCol32, 16, 1.5f);

    const ::Mouse::Type &curType{mouse.style.type};

    if (::Mouse::isTypeCircle(curType)) {
      draw->AddCircle(mouseCenter, state.layout.mouse.radius * zoom, fill, 64,
                      mouse.outerThickness);
    } else if (::Mouse::isTypeRect(curType)) {
      const ImVec2 topLeft(
          mouseCenter.x - (state.layout.mouse.width * 0.5f) * zoom,
          mouseCenter.y - (state.layout.mouse.height * 0.5f) * zoom);
      const ImVec2 bottomRight(
          mouseCenter.x + (state.layout.mouse.width * 0.5f) * zoom,
          mouseCenter.y + (state.layout.mouse.height * 0.5f) * zoom);
      draw->AddRect(topLeft, bottomRight, fill, mouse.rounding, 0,
                    mouse.outerThickness);
    }
    ImGui::PopID();
  }
  if (state.mouseCenterSelected &&
      ::Mouse::isTypeRect(app.config.theme.mouse.style.type)) {
    drawMouseRectResizeHandles(state, draw,
                               {origin.x + state.layout.mouse.centerX * zoom,
                                origin.y + state.layout.mouse.centerY * zoom});
  }

  ImGuiMultiSelectFlags msFlags{ImGuiMultiSelectFlags_BoxSelect2d |
                                ImGuiMultiSelectFlags_NoRangeSelect |
                                ImGuiMultiSelectFlags_ClearOnEscape |
                                ImGuiMultiSelectFlags_ClearOnClickVoid};

  ImGuiMultiSelectIO *msIo{ImGui::BeginMultiSelect(
      msFlags, state.selection.Size, (int)state.layout.keys.size())};
  state.selection.ApplyRequests(msIo);
  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    state.keysDragged = false;

  for (int i{0}; i < static_cast<int>(state.layout.keys.size()); ++i) {
    Key &key{state.layout.keys[i]};

    const ImVec2 keyPos{origin.x + key.x * zoom, origin.y + key.y * zoom};
    const ImVec2 keySize{key.w * zoom, key.h * zoom};
    const ImVec2 keyPosMax{keyPos.x + keySize.x, keyPos.y + keySize.y};
    const bool isSelected{state.selection.Contains((ImGuiID)i)};

    ImGui::PushID(i);
    ImGui::SetCursorScreenPos(keyPos);
    ImGui::SetNextItemSelectionUserData(i);
    ImGui::Selectable("##body", isSelected, ImGuiSelectableFlags_AllowOverlap,
                      keySize);

    if (ImGui::IsItemActivated()) {
      state.mouseCenterSelected = false;
      state.moveRaw = {key.x, key.y};
      state.moveAnchor = {key.x, key.y};
      saveUndoClearRedo(state);
    }
    if (state.selection.Size >= 1) {
      state.mouseCenterSelected = false;
    }

    if (ImGui::IsItemActive() &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      state.keysDragged = true;

      const ImVec2 mouseDelta{ImGui::GetIO().MouseDelta.x / zoom,
                              ImGui::GetIO().MouseDelta.y / zoom};
      const ImVec2 delta{computeDragDelta(state, mouseDelta)};

      if (state.selection.Contains(static_cast<ImGuiID>(i))) {
        moveSelectedKeys(state, delta);
      } else {
        key.x += delta.x;
        key.y += delta.y;
      }
    }

    const ImU32 fill{isSelected ? IM_COL32(80, 140, 255, 180)
                                : IM_COL32(60, 60, 60, 180)};
    const float rounding{app.config.theme.keyRounding * zoom};

    draw->AddRectFilled(keyPos, keyPosMax, fill, rounding);
    draw->AddRect(keyPos, keyPosMax, whiteCol32, rounding);

    const ImVec2 textSize{ImGui::CalcTextSize(key.label.c_str())};
    const ImVec2 textPos{keyPos.x + (keySize.x - textSize.x) * 0.5f,
                         keyPos.y + (keySize.y - textSize.y) * 0.5f};
    draw->AddText(textPos, whiteCol32, key.label.c_str());

    ImGui::PopID();
  }

  msIo = ImGui::EndMultiSelect();

  const bool releasedAfterDrag{state.keysDragged &&
                               ImGui::IsMouseReleased(ImGuiMouseButton_Left)};
  if (!releasedAfterDrag)
    state.selection.ApplyRequests(msIo);

  // const ::Mouse::Type
  if (state.selection.Size == 1 && !state.mouseCenterSelected) {
    void *it{nullptr};
    ImGuiID id{};
    if (state.selection.GetNextSelectedItem(&it, &id) &&
        id < state.layout.keys.size()) {
      Key &key{state.layout.keys[id]};
      const ImVec2 kMin{origin.x + key.x * zoom, origin.y + key.y * zoom};
      const ImVec2 kMax{kMin.x + key.w * zoom, kMin.y + key.h * zoom};
      drawResizeHandles(state, draw, key, kMin, kMax);
    }
  }
  ImGui::PopStyleColor();
  ImGui::PopFont();
}

static void drawAdvancedLabelButtons(LayoutEditState &state) {
  if (ImGui::Button("UPPER")) {
    changeSelectedLetterCase(state, true);
  }

  ImGui::SameLine();

  if (ImGui::Button("lower")) {
    changeSelectedLetterCase(state, false);
  }

  std::string suggestedLabelsButtonLabel{state.selection.Size > 1
                                             ? "Apply suggested labels"
                                             : "Apply suggested label"};
  if (ImGui::Button(suggestedLabelsButtonLabel.c_str())) {
    applySuggestedLabels(state);
  }

  std::string deleteLabelsButtonLabel{
      state.selection.Size > 1 ? "Delete labels" : "Delete label"};
  if (ImGui::Button(deleteLabelsButtonLabel.c_str())) {
    deleteLabels(state);
  }
}

void drawLayoutEditorProperties(LayoutEditState &state, const AppState &app) {
  constexpr float sliderWidth{150.0f};

  if (state.mouseCenterSelected) {
    ImGui::Text("Mouse center");
    const auto &mouseStyle{app.config.theme.mouse.style};

    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::DragFloat("X", &state.layout.mouse.centerX, state.step);
    if (ImGui::IsItemActivated())
      saveUndoClearRedo(state);

    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::DragFloat("Y", &state.layout.mouse.centerY, state.step);
    if (ImGui::IsItemActivated())
      saveUndoClearRedo(state);

    if (::Mouse::isTypeCircle(mouseStyle.type)) {
      ImGui::SetNextItemWidth(sliderWidth);
      ImGui::DragFloat("Radius", &state.layout.mouse.radius, state.step);
    }
    if (::Mouse::isTypeRect(mouseStyle.type)) {
      ImGui::SetNextItemWidth(sliderWidth);
      ImGui::DragFloat("Width", &state.layout.mouse.width, state.step);
      ImGui::SetNextItemWidth(sliderWidth);
      ImGui::DragFloat("Height", &state.layout.mouse.height, state.step);
    }
    return;
  }

  if (state.selection.Size == 0) {
    ImGui::TextDisabled("No keys selected");
    if (ImGui::Button("Add key")) {
      addKey(state);
    }
    if (ImGui::Button("Add special key")) {
      ImGui::OpenPopup("SpecialKeyPopup");
    }
    addSpecialKeyPopup(state);
    return;
  }

  void *it{nullptr};
  ImGuiID id;

  if (!state.selection.GetNextSelectedItem(&it, &id))
    return;

  int idx{static_cast<int>(id)};

  if (idx < 0 || idx >= static_cast<int>(state.layout.keys.size())) {
    state.selection.Clear();
    return;
  }

  Key &key{state.layout.keys[idx]};

  if (state.selection.Size == 1) {
    if (state.focusLabelInput) {
      ImGui::SetWindowFocus("Properties##overlay");
      ImGui::SetKeyboardFocusHere();
      state.focusLabelInput = false;
    }

    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::InputText("Label", &key.label);
    if (ImGui::IsItemActivated())
      saveUndoClearRedo(state);

    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::InputText("Shift", &key.shiftLabel);
    if (ImGui::IsItemActivated())
      saveUndoClearRedo(state);

    ImGui::SameLine();
    Helper::HelpMarker("Shift label");

    if (key.type == KeyType::Keyboard) {
      ImGui::Text("Key code: %d", key.code);
      ImGui::SameLine();
      if (ImGui::Button(state.recordingKeyCode ? "Press a key..."
                                               : "Record key")) {
        state.recordingKeyCode = true;
      }
    } else {
      ImGui::Text("Type: %s", std::find_if(sKeysInfo.begin(), sKeysInfo.end(),
                                           [&](const SpecialKey &sKey) {
                                             return sKey.type == key.type;
                                           })
                                  ->name.c_str());
    }

    recordKeyCode(state, key, app.keyboard);

  } else if (state.selection.Size > 1) {
    ImGui::Text("%d keys selected", state.selection.Size);
  }

  drawMultiKeyProperty(state, KeyProp::X, sliderWidth);
  drawMultiKeyProperty(state, KeyProp::Y, sliderWidth);
  drawMultiKeyProperty(state, KeyProp::W, sliderWidth);
  drawMultiKeyProperty(state, KeyProp::H, sliderWidth);
  drawMultiKeyProperty(state, KeyProp::gapX, sliderWidth);
  drawMultiKeyProperty(state, KeyProp::gapY, sliderWidth);

  if (state.showAdvancedLabelButtons) {
    drawAdvancedLabelButtons(state);
  }

  std::string deleteButtonLabel{state.selection.Size > 1 ? "Delete keys"
                                                         : "Delete key"};
  if (ImGui::Button(deleteButtonLabel.c_str())) {
    saveUndoClearRedo(state);
    deleteSelectedKeys(state);
  }
}

void drawLayoutPropertiesOverlay(LayoutEditState &state, AppState &app) {
  const ImVec2 display{ImGui::GetIO().DisplaySize};
  constexpr float pad{30.0f};
  constexpr float dockHeight{240.0f};

  ImGui::SetNextWindowPos(
      {(display.x - pad), (display.y - (display.y * 0.5f) - dockHeight)},
      ImGuiCond_Always, ImVec2(1.0f, 0.0f));
  ImGui::SetNextWindowSize({dockHeight, 0.0f});
  ImGui::SetNextWindowBgAlpha(0.5f);

  const ImGuiWindowFlags flags{
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize |
      ImGuiWindowFlags_NoFocusOnAppearing};
  ImGui::Begin("Properties##overlay", nullptr, flags);
  drawLayoutEditorProperties(state, app);
  ImGui::End();
}

static void editCurrentLayout(LayoutEditState &state, AppState &app) {
  saveUndoClearRedo(state);
  state.layout.keys.clear();
  state.layout = app.layout;
  state.selection.Clear();
  state.resetViewOffset();
  state.mouseCenterSelected = false;
  state.layoutNameInput = app.config.selectedLayout;
  state.defaultKeyHeight = mostFreqWH(state, KeyProp::H);
  state.defaultKeyWidth = mostFreqWH(state, KeyProp::W);
  const float mostFreqGapX = getMostFreqGapX(state);
  state.gap = mostFreqGapX;
  if (mostFreqGapX > 0.0f) {
    state.gridSize = mostFreqGapX;
  }
}
static void resetLayout(LayoutEditState &state) {
  saveUndoClearRedo(state);
  state.layout.keys.clear();
  state.selection.Clear();
  state.resetViewOffset();
  state.mouseCenterSelected = false;
}

static void saveLayoutPopup(LayoutEditState &state, AppState &app) {
  if (ImGui::BeginPopupModal("Save Layout", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {

    ImGui::Text("A layout with this name already exists.\n"
                "Do you want to overwrite it?");

    if (ImGui::Button("Yes", ImVec2(100, 0))) {
      state.layoutNameError = Helper::getFileCreateError(
          saveLayout(state.layoutNameInput, state.layout));

      if (state.layoutNameError.empty()) {
        refreshLayouts();

        if (state.layoutNameInput == app.config.selectedLayout) {
          loadLayout(state.layoutNameInput, app);
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
}

static void drawWindowLayoutOptions(LayoutEditState &state, AppState &app) {
  ImGui::BeginChild("Window & layout options", ImVec2(420, 170), true);

  if (ImGui::Button("Edit current layout")) {
    editCurrentLayout(state, app);
  }

  ImGui::SameLine();

  if (ImGui::Button("Reset layout")) {
    resetLayout(state);
  }

  if (ImGui::Button("Add key")) {
    addKey(state);
  }

  ImGui::SameLine();

  if (ImGui::Button("Add special key")) {
    ImGui::OpenPopup("SpecialKeyPopup");
  }
  addSpecialKeyPopup(state);

  ImGui::SetNextItemWidth(150);
  ImGui::InputText("##LayoutName", &state.layoutNameInput);
  ImGui::SameLine();

  if (ImGui::Button("Save layout")) {
    const auto path = Paths::getLayoutPath(state.layoutNameInput);

    if (std::filesystem::exists(path)) {
      ImGui::OpenPopup("Save Layout");
    } else {
      state.layoutNameError = Helper::getFileCreateError(
          saveLayout(state.layoutNameInput, state.layout));

      if (state.layoutNameError.empty()) {
        refreshLayouts();
      }
    }
  }

  if (!state.layoutNameError.empty()) {
    Helper::drawErrorInSettings(state.layoutNameError);
  }

  saveLayoutPopup(state, app);

  // todo make clamp between user monitor
  Helper::SliderInt("Window width", state.layout.windowWidth, 10, 1920, "%d",
                    260);

  Helper::SliderInt("Window height", state.layout.windowHeight, 10, 1080, "%d",
                    260);

  ImGui::EndChild();
}

static void drawKeysGeneralOptions(LayoutEditState &state) {
  ImGui::BeginChild("Keys general options", ImVec2(420, 170), true);

  ImGui::Checkbox("Apply gap with multi resize",
                  &state.applyGapWithMultiResize);

  ImGui::Checkbox("Show advanced label buttons",
                  &state.showAdvancedLabelButtons);

  ImGui::Checkbox("Record after adding key", &state.recordAfterAdding);
  ImGui::Checkbox("Focus label after recording", &state.focusLabelAfterRecord);

  ImGui::Checkbox("Enable grid", &state.gridEnabled);

  if (state.gridEnabled) {
    ImGui::SameLine();

    ImGui::Checkbox("Snap to grid", &state.snapToGrid);
    Helper::SliderFloat("Grid size", state.gridSize, 1.0f, 30.0f, 1.0f, 100.0f,
                        "%.2f", 260);
    // ImGui::SeparatorText("");
  }

  Helper::SliderFloat("GapX", state.gap, 0.0f, 30.0f, -20.0f, 100.0f, "%.2f",
                      260);

  Helper::SliderFloat("Step", state.step, 0.01f, 30.0f, 0.01f, 100.0f, "%.2f",
                      260);

  Helper::SliderFloat("Key width", state.defaultKeyWidth, 10.0f, 200.0f, "%.0f",
                      260);

  Helper::SliderFloat("Key height", state.defaultKeyHeight, 10.0f, 200.0f,
                      "%.0f", 260);

  // Helper::SliderFloat("Zoom", state.zoom, Consts::EDITOR_MINZOOM,
  //                     Consts::EDITOR_MAXZOOM, "%.2f", 260);

  ImGui::EndChild();
}

static void drawLayoutEditorOptions(LayoutEditState &state, AppState &app) {
  drawWindowLayoutOptions(state, app);
  ImGui::SameLine();
  drawKeysGeneralOptions(state);
}

void drawLayoutEditorTab(LayoutEditState &state, AppState &app) {
  ImDrawList *draw{ImGui::GetWindowDrawList()};

  drawLayoutEditorOptions(state, app);

  ImGui::SeparatorText("");
  ImGui::BeginChild("canvas", ImGui::GetContentRegionAvail(), false,
                    ImGuiWindowFlags_HorizontalScrollbar);
  drawLayoutEditorCanvas(state, app);
  ImGui::EndChild();
}
} // namespace Layouts

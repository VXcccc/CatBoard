#pragma once

#include "imgui.h"
#include <nlohmann/json.hpp>
#include <string>

namespace Layouts {

enum class KeyProp { X, Y, W, H, maxProps };
constexpr std::array<std::string, static_cast<std::size_t>(KeyProp::maxProps)>
    KeyPropName{"X", "Y", "W", "H"};

struct Key {
  int keyCode;
  std::string label;
  std::string shiftLabel;
  float x;
  float y;
  float w;
  float h;
};
bool operator==(const Key &k1, const Key &k2);
bool operator!=(const Key &k1, const Key &k2);

struct Mouse {
  float centerX{10.0f};
  float centerY{10.0f};
  float width{100.0f};
  float height{75.0f};
  float radius{37.0f};
};

struct Layout {
  int windowWidth;
  int windowHeight;
  Mouse mouse{};
  std::vector<Key> keys;
};

struct LayoutEditState {
  Layout layout{230, 230};
  bool recordingKeyCode{false};
  std::string layoutNameInput;

  std::vector<Layouts::Layout> undoLayouts{};
  std::vector<ImGuiSelectionBasicStorage> undoSelections{};
  std::vector<Layouts::Layout> redoLayouts{};
  std::vector<ImGuiSelectionBasicStorage> redoSelections{};

  bool mouseCenterSelected{false};
  std::string layoutNameError{};

  float gap{1.0f};
  float step{1.0f};
  float defaultKeyWidth{40.0f};
  float defaultKeyHeight{40.0f};

  bool focusLabelInput{false};

  ImGuiSelectionBasicStorage selection;
  bool moveWhenMultiResize{true};

  bool gridEnabled{true};
  bool snapToGrid{true};
  float gridSize{1.0f};
  float zoom{1.0f};

  ImVec2 ViewOffset{0.0f, 0.0f};
  void resetViewOffset() { ViewOffset = {0.0f, 0.0f}; }

  float resizeRaw{};
  float resizeFixed{};

  ImVec2 moveRaw{};
  ImVec2 moveAnchor{};
};

extern std::vector<std::string> layouts;
extern Layout activeLayout;
extern std::string defaultLayoutName;

bool installDefaultLayouts();
void refreshLayouts();
bool loadLayout(std::string_view name);
bool deleteLayout(std::string_view name);

void drawLayoutEditorTab(LayoutEditState &state);

void drawLayoutPropertiesOverlay(LayoutEditState &state);
} // namespace Layouts

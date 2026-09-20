#pragma once

#include "imgui.h"
#include <nlohmann/json.hpp>

namespace Layouts {

struct Key {
  int keyCode;
  std::string label;
  float x;
  float y;
  float w;
  float h;
};

struct Mouse {
  float mouseCenterX{0.0f};
  float mouseCenterY{0.0f};
};

struct Layout {
  int windowWidth;
  int windowHeight;
  Mouse mouse{};
  std::vector<Key> keys;
};

struct LayoutEditState {
  Layout layout{40, 40};
  int selectedKeyIndex{-1};
  bool recordingKeyCode{false};
  std::string layoutNameInput;

  std::vector<Layouts::Layout> undoLayouts{};
  std::vector<int> undoIndexes{};
  std::vector<Layouts::Layout> redoLayouts{};
  std::vector<int> redoIndexes{};

  bool mouseCenterSelected{false};
  std::string layoutNameError{};

  float gap{1.0f};
  float step{1.0f};
  float defaultKeyWidth{40.0f};
  float defaultKeyHeight{40.0f};

  bool focusLabelInput{false};

  ImGuiSelectionBasicStorage selection;
};

extern std::vector<std::string> layouts;
extern Layout activeLayout;
extern std::string defaultLayoutName;

bool installDefaultLayouts();
void refreshLayouts();
bool loadLayout(std::string_view name);
bool deleteLayout(std::string_view name);

void drawLayoutEditorTab(LayoutEditState &state);
} // namespace Layouts

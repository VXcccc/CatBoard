#pragma once

#include <nlohmann/json.hpp>

namespace Layouts{

struct Key {
  int keyCode;
  std::string label;
  float x;
  float y;
  float w;
  float h;
};

struct Mouse{
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
    Layout layout; 
    int selectedKeyIndex{-1};
    bool recordingKeyCode{false};
    std::string layoutNameInput;
    std::vector<Layouts::Layout> undoLayouts{};
    std::vector<Layouts::Layout> redoLayouts{};
    bool mouseCenterSelected{false};
};

extern std::vector<std::string> layouts;
extern Layout activeLayout;
extern std::string defaultLayoutName;


bool installDefaultLayouts();
void refreshLayouts();
bool loadLayout(std::string_view name);

// void drawLayoutEditorCanvas(LayoutEditState &state);
// void drawLayoutEditorProperties(LayoutEditState &state);
void drawLayoutEditorTab(LayoutEditState &state);
}

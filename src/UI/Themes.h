#pragma once

#include "imgui.h"
#include <array>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
namespace Paths {
std::filesystem::path getUIJsonPath();
}

namespace Themes {

enum Style { Dark, White, RoseQuartz, GruvBox, Catppuccin, Dracula, MaxStyles };
constexpr std::array<std::string, MaxStyles> styles{
    "Dark", "White", "RoseQuartz", "GruvBox", "Catppuccin", "Dracula",
};

inline int defaultTheme{static_cast<int>(Style::RoseQuartz)};
inline int activeTheme{defaultTheme};

// Forward declarations
inline void ImGuiDarkStyle();
inline void ImGuiWhiteStyle();
inline void ImGuiRoseQuartzStyle();
inline void ImGuiGruvboxHardStyle();
inline void SetupImGuiCatppuccinMochaStyle();
inline void ImGuiDraculaStyle();

inline void ApplyTheme(int theme) {
  ImGui::GetStyle() = ImGuiStyle();
  switch (theme) {
  case Dark:
    ImGuiDarkStyle();
    break;
  case White:
    ImGuiWhiteStyle();
    break;
  case RoseQuartz:
    ImGuiRoseQuartzStyle();
    break;
  case GruvBox:
    ImGuiGruvboxHardStyle();
    break;
  case Catppuccin:
    SetupImGuiCatppuccinMochaStyle();
    break;
  case Dracula:
    ImGuiDraculaStyle();
    break;
  default:
    ImGuiDarkStyle();
    break;
  }
}

inline void LoadUiSettings() {
  std::ifstream file(Paths::getUIJsonPath());
  if (!file)
    return;

  try {
    const json j = json::parse(file);
    const int theme = j.value("theme", activeTheme);
    if (theme >= 0 && theme < static_cast<int>(Style::MaxStyles))
      activeTheme = theme;
  } catch (...) {
    activeTheme = defaultTheme;
  }
}

inline bool SaveUiSettings() {
  const auto path{Paths::getUIJsonPath()};
  std::filesystem::create_directories(path.parent_path());

  std::ofstream file{path};
  if (!file)
    return false;

  file << nlohmann::json{{"theme", activeTheme}}.dump(4);
  return file.good();
}

inline void ImGuiRoseQuartzStyle() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImVec4 *colors = style.Colors;

  // --- 1. Sizing and Spacing (Soft & Round) ---
  style.WindowPadding = ImVec2(10.0f, 10.0f);
  style.FramePadding = ImVec2(6.0f, 4.0f);
  style.ItemSpacing = ImVec2(8.0f, 5.0f);
  style.ScrollbarSize = 14.0f;
  style.GrabMinSize = 12.0f;

  // --- 2. Borders & Rounding (Maximum Softness) ---
  // style.WindowRounding = 10.0f;
  style.ChildRounding = 6.0f;
  style.FrameRounding = 6.0f;
  style.PopupRounding = 6.0f;
  style.ScrollbarRounding = 12.0f;
  style.GrabRounding = 6.0f;
  style.TabRounding = 6.0f;

  style.WindowBorderSize = 1.0f;
  style.FrameBorderSize = 0.0f;

  // --- 3. The Rose Quartz Palette ---

  // Text
  colors[ImGuiCol_Text] = ImVec4(0.95f, 0.90f, 0.95f, 1.00f); // Soft white-pink
  colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.45f, 0.55f, 1.00f);

  // Backgrounds
  colors[ImGuiCol_WindowBg] =
      ImVec4(0.12f, 0.10f, 0.12f, 1.00f); // Deep Plum-Grey
  colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.12f, 0.14f, 1.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.08f, 0.10f, 0.96f);

  // Borders
  colors[ImGuiCol_Border] = ImVec4(0.35f, 0.25f, 0.35f, 0.50f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

  // Frames
  colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.15f, 0.20f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.22f, 0.30f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.40f, 0.28f, 0.40f, 1.00f);

  // Title Bars
  colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.10f, 0.15f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.25f, 0.15f, 0.25f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.06f, 0.08f, 1.00f);

  // Menus
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.10f, 0.15f, 1.00f);

  // Scrollbars
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.06f, 0.08f, 1.00f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.25f, 0.40f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.55f, 0.35f, 0.55f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.70f, 0.45f, 0.70f, 1.00f);

  // Interactables (The Rose Pop)
  colors[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.60f, 0.75f, 1.00f); // Rose Pink
  colors[ImGuiCol_SliderGrab] = ImVec4(0.85f, 0.50f, 0.65f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.95f, 0.60f, 0.75f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.45f, 0.25f, 0.35f, 1.00f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.65f, 0.35f, 0.50f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.85f, 0.45f, 0.65f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.45f, 0.25f, 0.35f, 1.00f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.55f, 0.30f, 0.45f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.65f, 0.35f, 0.55f, 1.00f);

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.15f, 0.20f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.65f, 0.35f, 0.50f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.45f, 0.25f, 0.35f, 1.00f);

  // Misc
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.95f, 0.60f, 0.75f, 0.35f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.95f, 0.60f, 0.75f, 1.00f);

#ifdef IMGUI_HAS_DOCK
  colors[ImGuiCol_DockingPreview] = ImVec4(0.95f, 0.60f, 0.75f, 0.40f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.12f, 0.10f, 0.12f, 1.00f);
#endif
}

inline void ImGuiDarkStyle() {
  ImGui::StyleColorsDark();

  ImGuiStyle &style = ImGui::GetStyle();

  // --- 1. Sizing and Spacing (Modern & Tight) ---
  style.WindowPadding = ImVec2(8.0f, 8.0f);
  style.FramePadding = ImVec2(5.0f, 3.0f);
  style.CellPadding = ImVec2(6.0f, 4.0f);
  style.ItemSpacing = ImVec2(6.0f, 4.0f);
  style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
  style.ScrollbarSize = 13.0f;
  style.GrabMinSize = 10.0f;

  // --- 2. Borders & Rounding ---
  style.WindowBorderSize = 1.0f;
  style.ChildBorderSize = 1.0f;
  style.PopupBorderSize = 1.0f;
  style.FrameBorderSize = 1.0f;

  // style.WindowRounding = 4.0f;
  style.ChildRounding = 3.0f;
  style.FrameRounding = 3.0f;
  style.PopupRounding = 3.0f;
  style.ScrollbarRounding = 9.0f;
  style.GrabRounding = 3.0f;
  style.TabRounding = 3.0f;
}

inline void ImGuiWhiteStyle() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImVec4 *colors = style.Colors;

  // --- 1. Sizing & Spacing (Clean & Rigid) ---
  style.WindowPadding = ImVec2(12.0f, 12.0f);
  style.FramePadding = ImVec2(6.0f, 4.0f);
  style.CellPadding = ImVec2(6.0f, 4.0f);
  style.ItemSpacing = ImVec2(8.0f, 6.0f);
  style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
  style.ScrollbarSize = 14.0f;
  style.GrabMinSize = 12.0f;

  // --- 2. Borders & Rounding (Technical/Drafting feel) ---
  // style.WindowRounding = 2.0f;
  style.ChildRounding = 2.0f;
  style.FrameRounding = 2.0f;
  style.PopupRounding = 2.0f;
  style.ScrollbarRounding = 12.0f;
  style.GrabRounding = 2.0f;
  style.TabRounding = 2.0f;

  style.WindowBorderSize = 1.0f;
  style.ChildBorderSize = 1.0f;
  style.PopupBorderSize = 1.0f;
  style.FrameBorderSize = 1.0f;
  style.TabBorderSize = 1.0f;

  // --- 3. Full Color Palette ---

  // Main Text & Background
  colors[ImGuiCol_Text] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); // Deep Carbon Ink
  colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.96f, 0.96f, 0.94f, 1.00f); // Warm Paper
  colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.03f);
  colors[ImGuiCol_PopupBg] =
      ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // Clean White Popups

  // Borders & Separators
  colors[ImGuiCol_Border] = ImVec4(0.75f, 0.75f, 0.72f, 1.00f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_Separator] = ImVec4(0.80f, 0.80f, 0.78f, 1.00f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.17f, 0.34f, 0.59f, 0.78f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);

  // Frames (Inputs, Checkboxes, etc)
  colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.92f, 0.95f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.85f, 0.88f, 0.92f, 1.00f);

  // Titles & Menus
  colors[ImGuiCol_TitleBg] = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.88f, 0.88f, 0.86f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.92f, 0.92f, 0.90f, 0.75f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);

  // Scrollbars
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.96f, 0.96f, 0.94f, 1.00f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.78f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.70f, 0.70f, 0.68f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.60f, 0.60f, 0.58f, 1.00f);

  // Interactables (Blueprint Blue)
  colors[ImGuiCol_CheckMark] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.17f, 0.34f, 0.59f, 0.70f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.17f, 0.34f, 0.59f, 0.08f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.17f, 0.34f, 0.59f, 0.20f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.17f, 0.34f, 0.59f, 0.35f);

  // Header (Selection in lists/trees)
  colors[ImGuiCol_Header] = ImVec4(0.17f, 0.34f, 0.59f, 0.12f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.17f, 0.34f, 0.59f, 0.25f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.17f, 0.34f, 0.59f, 0.40f);

  // Tables (Crucial for Light Mode)
  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.90f, 0.90f, 0.88f, 1.00f);
  colors[ImGuiCol_TableBorderStrong] = ImVec4(0.75f, 0.75f, 0.72f, 1.00f);
  colors[ImGuiCol_TableBorderLight] = ImVec4(0.85f, 0.85f, 0.82f, 1.00f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.00f, 0.00f, 0.00f, 0.03f);

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.96f, 0.96f, 0.94f, 1.00f);

  // Misc
  colors[ImGuiCol_PlotLines] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.17f, 0.34f, 0.59f, 0.25f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(0.17f, 0.34f, 0.59f, 0.90f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);

#ifdef IMGUI_HAS_DOCK
  colors[ImGuiCol_DockingPreview] = ImVec4(0.17f, 0.34f, 0.59f, 0.40f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.96f, 0.96f, 0.94f, 1.00f);
#endif
}

inline void ImGuiGruvboxHardStyle() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImVec4 *colors = style.Colors;

  // --- 1. Sizing and Spacing (Industrial & Square) ---
  style.WindowPadding = ImVec2(10.0f, 10.0f);
  style.FramePadding = ImVec2(6.0f, 4.0f);
  style.ItemSpacing = ImVec2(8.0f, 4.0f);
  style.ScrollbarSize = 14.0f;
  style.GrabMinSize = 12.0f;

  // --- 2. Borders & Rounding (Gruvbox usually looks best with sharp or low
  // rounding) --- style.WindowRounding = 2.0f;
  style.FrameRounding = 2.0f;
  style.PopupRounding = 2.0f;
  style.ScrollbarRounding = 2.0f;
  style.GrabRounding = 2.0f;
  style.TabRounding = 2.0f;

  style.WindowBorderSize = 1.0f;
  style.FrameBorderSize = 1.0f;
  style.PopupBorderSize = 1.0f;

  // --- 3. The Gruvbox Dark Hard Palette ---
  // Background: #1d2021 (Dark Hard) | Foreground: #ebdbb2
  // Red: #fb4934 | Green: #b8bb26 | Yellow: #fabd2f | Blue: #83a598
  // Gray: #928374 | Orange: #fe8019

  // Text
  colors[ImGuiCol_Text] = ImVec4(0.92f, 0.86f, 0.70f, 1.00f);         // #ebdbb2
  colors[ImGuiCol_TextDisabled] = ImVec4(0.57f, 0.51f, 0.45f, 1.00f); // #928374

  // Backgrounds
  colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.13f, 0.13f, 1.00f); // #1d2021
  colors[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.13f, 0.13f, 0.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.13f, 0.13f, 0.95f);

  // Borders
  colors[ImGuiCol_Border] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f); // #504945
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

  // Frames
  colors[ImGuiCol_FrameBg] = ImVec4(0.24f, 0.22f, 0.21f, 1.00f); // #3c3836
  colors[ImGuiCol_FrameBgHovered] =
      ImVec4(0.31f, 0.29f, 0.27f, 1.00f); // #504945
  colors[ImGuiCol_FrameBgActive] =
      ImVec4(0.40f, 0.36f, 0.33f, 1.00f); // #665c54

  // Title Bars
  colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.13f, 0.13f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.11f, 0.13f, 0.13f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.11f, 0.13f, 0.13f, 1.00f);

  // Menus
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.14f, 0.13f, 1.00f); // #282828

  // Scrollbars
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.11f, 0.13f, 0.13f, 1.00f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.36f, 0.33f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.57f, 0.51f, 0.45f, 1.00f);

  // Interactables
  colors[ImGuiCol_CheckMark] =
      ImVec4(0.72f, 0.73f, 0.15f, 1.00f); // #b8bb26 (Green)
  colors[ImGuiCol_SliderGrab] =
      ImVec4(0.51f, 0.65f, 0.60f, 1.00f); // #83a598 (Blue)
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.55f, 0.73f, 0.67f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
  colors[ImGuiCol_ButtonHovered] =
      ImVec4(0.98f, 0.29f, 0.20f, 1.00f); // #fb4934 (Red)
  colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.20f, 0.15f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.24f, 0.22f, 0.21f, 1.00f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.36f, 0.33f, 1.00f);

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4(0.24f, 0.22f, 0.21f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);

  // Misc
  colors[ImGuiCol_PlotLines] =
      ImVec4(0.98f, 0.74f, 0.18f, 1.00f); // #fabd2f (Yellow)
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.98f, 0.29f, 0.20f, 1.00f);

#ifdef IMGUI_HAS_DOCK
  colors[ImGuiCol_DockingPreview] = ImVec4(0.72f, 0.73f, 0.15f, 0.50f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.11f, 0.13f, 0.13f, 1.00f);
#endif
}

inline void SetupImGuiCatppuccinMochaStyle() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImVec4 *colors = style.Colors;

  // --- 1. Sizing and Spacing (Soft & Modern) ---
  style.WindowPadding = ImVec2(12.0f, 12.0f);
  style.FramePadding = ImVec2(6.0f, 4.0f);
  style.ItemSpacing = ImVec2(8.0f, 6.0f);
  style.ScrollbarSize = 14.0f;
  style.GrabMinSize = 12.0f;

  // --- 2. Borders & Rounding ---
  // style.WindowRounding = 8.0f;
  style.FrameRounding = 5.0f;
  style.PopupRounding = 5.0f;
  style.ScrollbarRounding = 12.0f;
  style.GrabRounding = 5.0f;
  style.TabRounding = 5.0f;

  style.WindowBorderSize = 1.0f;
  style.FrameBorderSize = 0.0f; // Minimalist look
  style.PopupBorderSize = 1.0f;

  // --- 3. The Catppuccin Mocha Palette ---
  // Base: #1e1e2e | Mantle: #181825 | Crust: #11111b
  // Text: #cdd6f4 | Subtext0: #a6adc8 | Surface0: #313244
  // Lavender: #b4befe | Sapphire: #74c7ec | Mauve: #cba6f7

  // Text
  colors[ImGuiCol_Text] = ImVec4(0.80f, 0.84f, 0.96f, 1.00f); // Text
  colors[ImGuiCol_TextDisabled] =
      ImVec4(0.42f, 0.45f, 0.55f, 1.00f); // Surface1

  // Backgrounds
  colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f); // Base
  colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);  // Mantle
  colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.11f, 0.96f);  // Crust

  // Borders
  colors[ImGuiCol_Border] = ImVec4(0.19f, 0.20f, 0.27f, 1.00f); // Surface0
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

  // Frames (Inputs, etc.)
  colors[ImGuiCol_FrameBg] = ImVec4(0.19f, 0.20f, 0.27f, 1.00f); // Surface0
  colors[ImGuiCol_FrameBgHovered] =
      ImVec4(0.25f, 0.26f, 0.35f, 1.00f); // Surface1
  colors[ImGuiCol_FrameBgActive] =
      ImVec4(0.31f, 0.32f, 0.42f, 1.00f); // Surface2

  // Title Bars
  colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);       // Mantle
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f); // Base
  colors[ImGuiCol_TitleBgCollapsed] =
      ImVec4(0.07f, 0.07f, 0.11f, 1.00f); // Crust

  // Menus
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);

  // Scrollbars
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
  colors[ImGuiCol_ScrollbarGrab] =
      ImVec4(0.31f, 0.32f, 0.42f, 1.00f); // Surface2
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.37f, 0.38f, 0.51f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.42f, 0.45f, 0.55f, 1.00f);

  // Interactables
  colors[ImGuiCol_CheckMark] = ImVec4(0.71f, 0.75f, 1.00f, 1.00f);  // Lavender
  colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.78f, 0.93f, 1.00f); // Sapphire
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.78f, 0.93f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.19f, 0.20f, 0.27f, 1.00f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.65f, 0.97f, 1.00f); // Mauve
  colors[ImGuiCol_ButtonActive] = ImVec4(0.70f, 0.55f, 0.87f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.19f, 0.20f, 0.27f, 1.00f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.26f, 0.35f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.31f, 0.32f, 0.42f, 1.00f);

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.31f, 0.32f, 0.42f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.20f, 0.27f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);

  // Misc
  colors[ImGuiCol_PlotLines] = ImVec4(0.94f, 0.72f, 0.42f, 1.00f); // Marigold
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.31f, 0.32f, 0.42f, 1.00f);
  colors[ImGuiCol_NavHighlight] =
      ImVec4(0.71f, 0.75f, 1.00f, 1.00f); // Lavender

#ifdef IMGUI_HAS_DOCK
  colors[ImGuiCol_DockingPreview] = ImVec4(0.71f, 0.75f, 1.00f, 0.50f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
#endif
}

inline void ImGuiDraculaStyle() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImVec4 *colors = style.Colors;

  // --- 1. Sizing and Spacing (Clean & Balanced) ---
  style.WindowPadding = ImVec2(10.0f, 10.0f);
  style.FramePadding = ImVec2(6.0f, 4.0f);
  style.ItemSpacing = ImVec2(8.0f, 6.0f);
  style.ScrollbarSize = 14.0f;
  style.GrabMinSize = 12.0f;

  // --- 2. Borders & Rounding ---
  // style.WindowRounding = 6.0f;
  style.FrameRounding = 4.0f;
  style.PopupRounding = 4.0f;
  style.ScrollbarRounding = 12.0f;
  style.GrabRounding = 4.0f;
  style.TabRounding = 4.0f;

  style.WindowBorderSize = 1.0f;
  style.FrameBorderSize = 1.0f;

  // --- 3. The Dracula Color Palette ---
  // Background: #282a36 | Selection: #44475a | Foreground: #f8f8f2
  // Comment: #6272a4    | Cyan: #8be9fd      | Green: #50fa7b
  // Orange: #ffb86c     | Pink: #ff79c6      | Purple: #bd93f9
  // Red: #ff5555        | Yellow: #f1fa8c

  // Text
  colors[ImGuiCol_Text] = ImVec4(0.97f, 0.97f, 0.95f, 1.00f);         // #f8f8f2
  colors[ImGuiCol_TextDisabled] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f); // #6272a4

  // Backgrounds
  colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f); // #282a36
  colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.16f, 0.21f, 0.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.16f, 0.16f, 0.21f, 0.96f);

  // Borders
  colors[ImGuiCol_Border] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f); // #44475a
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

  // Frames (Inputs, etc.)
  colors[ImGuiCol_FrameBg] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f); // #44475a
  colors[ImGuiCol_FrameBgHovered] =
      ImVec4(0.38f, 0.45f, 0.64f, 1.00f); // #6272a4
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.48f, 0.55f, 0.74f, 1.00f);

  // Title Bars
  colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f); // Darker
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);

  // Menus
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);

  // Scrollbars
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.48f, 0.55f, 0.74f, 1.00f);

  // Interactables
  colors[ImGuiCol_CheckMark] =
      ImVec4(0.31f, 0.98f, 0.48f, 1.00f); // #50fa7b (Green)
  colors[ImGuiCol_SliderGrab] =
      ImVec4(0.74f, 0.58f, 0.98f, 1.00f); // #bd93f9 (Purple)
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.84f, 0.68f, 1.00f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
  colors[ImGuiCol_ButtonHovered] =
      ImVec4(1.00f, 0.47f, 0.78f, 1.00f); // #ff79c6 (Pink)
  colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.37f, 0.62f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.55f, 0.74f, 1.00f);

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);

  // Tables
  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
  colors[ImGuiCol_TableBorderStrong] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
  colors[ImGuiCol_TableBorderLight] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);

  // Misc
  colors[ImGuiCol_PlotLines] =
      ImVec4(0.55f, 0.91f, 0.99f, 1.00f); // #8be9fd (Cyan)
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.74f, 0.58f, 0.98f, 1.00f);

#ifdef IMGUI_HAS_DOCK
  colors[ImGuiCol_DockingPreview] = ImVec4(0.74f, 0.58f, 0.98f, 0.50f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
#endif
}

} // namespace Themes

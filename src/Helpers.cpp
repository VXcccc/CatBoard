#include "Helpers.h"
#include "Config.h"
#include "Constants.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <winuser.h>

namespace Helper {

[[noreturn]] void fatalError(const wchar_t *message) {
  MessageBox(NULL, message, L"Catboard Fatal Error", MB_OK | MB_ICONERROR);
  std::exit(EXIT_FAILURE);
}

void errorMesBox(const wchar_t *message) {
  MessageBox(NULL, message, L"Catboard Error", MB_OK | MB_ICONERROR);
}

void drawErrorInSettings(std::string &error) {
  static double hideAt = 0.0;
  static std::string lastError;

  if (error.empty()) {
    lastError.clear();
    hideAt = 0.0;
    return;
  }
  if (error != lastError) {
    lastError = error;
    hideAt = ImGui::GetTime() + Consts::ERROR_DELETE_TIME;
  }
  if (ImGui::GetTime() >= hideAt) {
    error.clear();
    lastError.clear();
    hideAt = 0.0;
    return;
  }

  ImVec4 redColor{1, 0.2f, 0.2f, 1};
  ImGui::PushStyleColor(ImGuiCol_Text, redColor);
  ImGui::TextUnformatted(error.c_str());
  ImGui::PopStyleColor();
}

int toColorRGB(float value) {
  value = std::clamp(value, 0.0f, 1.0f);
  return static_cast<int>(std::round(value * 255.0f));
}

float fromColorRGB(int value) {
  value = std::clamp(value, 0, 255);
  return static_cast<float>(value) / 255.0f;
}

Config::Theme::ColorRGBA toConfigColor(const ImVec4 &color) {
  return Config::Theme::ColorRGBA{
      Helper::toColorRGB(color.x), Helper::toColorRGB(color.y),
      Helper::toColorRGB(color.z), Helper::toColorRGB(color.w)};
}

ImVec4 fromConfigColor(const Config::Theme::ColorRGBA &color) {
  return ImVec4{Helper::fromColorRGB(color[0]), Helper::fromColorRGB(color[1]),
                Helper::fromColorRGB(color[2]), Helper::fromColorRGB(color[3])};
}

void refresh(std::vector<std::string> &category,
             const std::filesystem::path &folder, std::string_view ext) {
  category.clear();

  if (!std::filesystem::exists(folder))
    return;

  for (const auto &entry : std::filesystem::directory_iterator(folder)) {
    if (!entry.is_regular_file())
      continue;

    if (entry.path().extension() != ext)
      continue;

    category.push_back(entry.path().stem().string());
  }
}

std::string getFileCreateError(Helper::FileCreateResult result) {
  switch (result) {
  case Helper::FileCreateResult::Success:
    return {};

  case Helper::FileCreateResult::EmptyName:
    return "Name cannot be empty";

  case Helper::FileCreateResult::NameTooLong:
    return "Name is too long";

  case Helper::FileCreateResult::AlreadyExists:
    return "File already exists";

  case Helper::FileCreateResult::DirectoryError:
    return "Could not create or find directory";

  case Helper::FileCreateResult::FileError:
    return "Could not create file";

  default:
    return "Unknown error";
  };
}

std::string VkToName(int vk) {
  static const std::unordered_map<int, std::string> map{
      {VK_LSHIFT, "L Shift"},  {VK_RSHIFT, "R Shift"}, {VK_LCONTROL, "L Ctrl"},
      {VK_RCONTROL, "R Ctrl"}, {VK_LMENU, "L Alt"},    {VK_RMENU, "R Alt"},

      {VK_CAPITAL, "Caps"},    {VK_TAB, "Tab"},        {VK_RETURN, "Enter"},
      {VK_BACK, "Backspace"},  {VK_SPACE, "Space"},    {VK_ESCAPE, "Escape"},
      {VK_DELETE, "Delete"},   {VK_INSERT, "Insert"},  {VK_HOME, "Home"},
      {VK_END, "End"},         {VK_PRIOR, "PageUp"},   {VK_NEXT, "PageDown"},

      {VK_UP, "Up"},           {VK_DOWN, "Down"},      {VK_LEFT, "Left"},
      {VK_RIGHT, "Right"},

      {VK_F1, "F1"},           {VK_F2, "F2"},          {VK_F3, "F3"},
      {VK_F4, "F4"},           {VK_F5, "F5"},          {VK_F6, "F6"},
      {VK_F7, "F7"},           {VK_F8, "F8"},          {VK_F9, "F9"},
      {VK_F10, "F10"},         {VK_F11, "F11"},        {VK_F12, "F12"},

      {VK_NUMPAD0, "Num0"},    {VK_NUMPAD1, "Num1"},   {VK_NUMPAD2, "Num2"},
      {VK_NUMPAD3, "Num3"},    {VK_NUMPAD4, "Num4"},   {VK_NUMPAD5, "Num5"},
      {VK_NUMPAD6, "Num6"},    {VK_NUMPAD7, "Num7"},   {VK_NUMPAD8, "Num8"},
      {VK_NUMPAD9, "Num9"},

      {VK_ADD, "Num+"},        {VK_SUBTRACT, "Num-"},  {VK_MULTIPLY, "Num*"},
      {VK_DIVIDE, "Num/"},     {VK_DECIMAL, "Num."},

      {VK_OEM_1, ";"},         {VK_OEM_PLUS, "="},     {VK_OEM_COMMA, ","},
      {VK_OEM_MINUS, "-"},     {VK_OEM_PERIOD, "."},   {VK_OEM_2, "/"},
      {VK_OEM_3, "`"},         {VK_OEM_4, "["},        {VK_OEM_5, "\\"},
      {VK_OEM_6, "]"},         {VK_OEM_7, "'"},
  };

  const auto it = map.find(vk);

  if (it != map.end())
    return it->second;

  // A-Z
  if (vk >= 'A' && vk <= 'Z')
    return std::string{static_cast<char>(vk)};

  // 0-9
  if (vk >= '0' && vk <= '9')
    return std::string{static_cast<char>(vk)};

  return "Unknown";
}

} // namespace Helper

#pragma once

// #include "Config/Config.h"
#include "imgui.h"
#include <array>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>
#include <windows.h>

namespace Config {
using ColorRGBA = std::array<int, 4>;
}

namespace Helper {
enum class FileCreateResult {
  Success,
  EmptyName,
  NameTooLong,
  AlreadyExists,
  DirectoryError,
  FileError,
  max_FileCreateResult
};

class CustomException : public std::exception {
public:
  CustomException(const char *msg) : m_message(msg) {}

  const char *what() const noexcept { return m_message.c_str(); }

private:
  std::string m_message;
};

std::wstring strToWide(const std::string &str);

[[noreturn]] void fatalError(const wchar_t *message);
void errorMesBox(const wchar_t *message);
void errorMesBox(const std::string &str);

void drawErrorInSettings(std::string &error);

void HelpMarker(const char *desc);

void refresh(std::vector<std::string> &category,
             const std::filesystem::path &folder, std::string_view ext);

void openReleasesPage();

std::string VkToName(int vk);

std::string VkToShiftLabel(int vk);

int toColorRGB(float value);
float fromColorRGB(int value);

Config::ColorRGBA toConfigColor(const ImVec4 &color);

ImVec4 fromConfigColor(const Config::ColorRGBA &color);

ImU32 ToImU32(const Config::ColorRGBA &c);

std::string getFileCreateError(Helper::FileCreateResult result);

bool isWindowsThemeLight();
void applyDarkMode(HWND hwnd);

// deprecated
// bool installDefault(const std::filesystem::path &source,
//                     const std::filesystem::path &destination);

bool ColorEdit(const std::string &label, Config::ColorRGBA &outCol,
               ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs);

// c_min, c_max = clamp min, clamp max
bool SliderInt(const std::string &label, int &v, int v_min, int v_max,
               int c_min, int c_max, const char *format = "%d",
               float sliderWidth = 0, ImGuiSliderFlags flags = 0);

// clamp default initialized with v_min, v_max
inline bool SliderInt(const std::string &label, int &v, int v_min, int v_max,
                      const char *format = "%d", float sliderWidth = 0,
                      ImGuiSliderFlags flags = 0) {
  return SliderInt(label, v, v_min, v_max, v_min, v_max, format, sliderWidth,
                   flags);
}

// c_min, c_max = clamp min, clamp max
bool SliderFloat(const std::string &label, float &v, float v_min, float v_max,
                 float c_min, float c_max, const char *format = "%.2f",
                 float sliderWidth = 0, ImGuiSliderFlags flags = 0);

// clamp default initialized with v_min, v_max
inline bool SliderFloat(const std::string &label, float &v, float v_min,
                        float v_max, const char *format = "%.2f",
                        float sliderWidth = 0, ImGuiSliderFlags flags = 0) {
  return SliderFloat(label, v, v_min, v_max, v_min, v_max, format, sliderWidth,
                     flags);
}

template <typename... Args>
void drawDebugText(const std::string &Format, Args &&...args) {
  char buffer[96];
  snprintf(buffer, sizeof(buffer), Format.c_str(), std::forward<Args>(args)...);

  ImVec4 yellowColor{1.0f, 1.0f, 0.0f, 1.0f};
  ImGui::PushStyleColor(ImGuiCol_Text, yellowColor); // yellow
  ImGui::TextUnformatted(buffer);
  ImGui::PopStyleColor();
}

template <typename T>
concept HasName = requires(const T &t) {
  { t.name } -> std::convertible_to<std::string>;
};

template <HasName T, std::size_t size>
std::vector<std::string> extractNames(const std::array<T, size> &arr) {
  std::vector<std::string> result{};
  result.reserve(size);
  for (const auto &style : arr) {
    result.emplace_back(style.name);
  }
  return result;
}

template <typename T, std::size_t size>
std::vector<T> arrToVec(const std::array<T, size> &arr) {
  std::vector<T> vec{};
  vec.reserve(size);
  vec.insert(vec.end(), arr.begin(), arr.end());
  return vec;
}

// P.S. i overloaded this function for enums
template <typename T> bool RadioEnum(const char *label, T *v, T v_button) {
  const bool pressed = ImGui::RadioButton(label, *v == v_button);
  if (pressed)
    *v = v_button;
  return pressed;
}

}; // namespace Helper

inline bool operator==(const ImVec2 &a, const ImVec2 &b) {
  return a.x == b.x && a.y == b.y;
}
inline bool operator!=(const ImVec2 &a, const ImVec2 &b) { return !(a == b); }

inline bool operator>(const ImVec2 &a, const ImVec2 &b) {
  return a.x > b.x && a.y > b.y;
}

inline ImVec2 operator*(const ImVec2 &vec, float num) {
  return ImVec2(vec.x * num, vec.y * num);
}

inline bool operator<(const ImVec2 &a, const ImVec2 &b) { return !(a > b); }

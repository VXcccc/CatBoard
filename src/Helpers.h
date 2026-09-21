#ifndef HELPERS_H
#define HELPERS_H

#include "Config.h"
#include "imgui.h"
#include <filesystem>
#include <string>
#include <vector>

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

[[noreturn]] void fatalError(const wchar_t *message);
void errorMesBox(const wchar_t *message);

void drawErrorInSettings(std::string &error);

void refresh(std::vector<std::string> &category,
             const std::filesystem::path &folder, std::string_view ext);

std::string VkToName(int vk);

int toColorRGB(float value);
float fromColorRGB(int value);

Config::Theme::ColorRGBA toConfigColor(const ImVec4 &color);

ImVec4 fromConfigColor(const Config::Theme::ColorRGBA &color);

std::string getFileCreateError(Helper::FileCreateResult result);

template <typename... Args>
void drawDebugText(const std::string &Format, Args &&...args) {
  char buffer[96];
  snprintf(buffer, sizeof(buffer), Format.c_str(), std::forward<Args>(args)...);

  ImVec4 yellowColor{1.0f, 1.0f, 0.0f, 1.0f};
  ImGui::PushStyleColor(ImGuiCol_Text, yellowColor); // yellow
  ImGui::TextUnformatted(buffer);
  ImGui::PopStyleColor();
}
}; // namespace Helper

#endif // !HELPERS_H

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

void refresh(std::vector<std::string> &category,
             const std::filesystem::path &folder, std::string_view ext);

std::string VkToName(int vk);

int toColorRGB(float value);
float fromColorRGB(int value);

Config::Theme::ColorRGBA toConfigColor(const ImVec4 &color);

ImVec4 fromConfigColor(const Config::Theme::ColorRGBA &color);

std::string getFileCreateError(Helper::FileCreateResult result);

}; // namespace Helper

#endif // !HELPERS_H

#ifndef HELPERS_H
#define HELPERS_H

#include "Config/Config.h"
#include "imgui.h"
#include <exception>
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

class CustomException : public std::exception {
public:
  CustomException(const char *msg) : m_message(msg) {}

  const char *what() const noexcept { return m_message.c_str(); }

private:
  std::string m_message;
};

[[noreturn]] void fatalError(const wchar_t *message);
void errorMesBox(const wchar_t *message);

void drawErrorInSettings(std::string &error);

void refresh(std::vector<std::string> &category,
             const std::filesystem::path &folder, std::string_view ext);

std::string VkToName(int vk);

int toColorRGB(float value);
float fromColorRGB(int value);

Config::ColorRGBA toConfigColor(const ImVec4 &color);

ImVec4 fromConfigColor(const Config::ColorRGBA &color);

std::string getFileCreateError(Helper::FileCreateResult result);

bool installDefault(const std::filesystem::path &source,
                    const std::filesystem::path &destination);

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

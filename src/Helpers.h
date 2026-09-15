#ifndef HELPERS_H
#define HELPERS_H

#include <filesystem>
#include <string>
#include <vector>

namespace Helper {
[[noreturn]] void fatalError(const wchar_t *message);
void errorMesBox(const wchar_t *message);


void refresh(std::vector<std::string> &category,
             const std::filesystem::path &folder, std::string_view ext);

std::string VkToName(int vk);

}; // namespace Helper

#endif // !HELPERS_H

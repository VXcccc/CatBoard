#ifndef HELPERS_H
#define HELPERS_H

#include <string>

namespace Helper {
[[noreturn]] void fatalError(const wchar_t *message);
void errorMesBox(const wchar_t *message);
std::string VkToName(int vk);
}; // namespace Helper

#endif // !HELPERS_H

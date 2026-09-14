#ifndef PATHS_H
#define PATHS_H

#include <filesystem>
#include <string_view>

namespace Paths {

std::filesystem::path getAppDataDir();

std::filesystem::path getExecutableDir();

std::filesystem::path getActiveConfigPath();

std::filesystem::path configDirectory();
std::filesystem::path layoutDirectory();
std::filesystem::path soundsDirectory();
std::filesystem::path fontsDirectory();

std::filesystem::path getConfigPath(std::string_view name);
std::filesystem::path getLayoutPath(std::string_view name);

std::filesystem::path getSoundPackDir(std::string_view name);
std::filesystem::path getSoundPackJsonPath(std::string_view name);

std::filesystem::path getFontPath(std::string_view name);

bool ensureConfigDirectory();
bool ensureLayoutDirectory();
bool ensureSoundsDirectory();
bool ensureFontsDirectory();

void openConfigDirectory();
void openLayoutDirectory();
void openSoundsDirectory();
void openFontsDirectory();

} // namespace Paths

#endif // !PATHS_H

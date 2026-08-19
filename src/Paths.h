#ifndef PATHS_H
#define PATHS_H

#include <filesystem>
#include <string_view>

namespace Paths {
std::filesystem::path getExecutableDir();

std::filesystem::path getAppDataDir();

std::filesystem::path getActiveConfigPath();
std::filesystem::path getActiveLayoutPath();

std::filesystem::path appDataConfigsDirPath();
std::filesystem::path appDataLayoutsDirPath();

/// @param name Config name without extension.
std::filesystem::path getConfigPath(std::string_view name);

/// @param name Layout name without extension.
std::filesystem::path getLayoutPath(std::string_view name);


std::filesystem::path configDirectory();
std::filesystem::path layoutDirectory();

bool ensureConfigDirectory();
bool ensureLayoutDirectory();

/// Open config folder in explorer
void openConfigDirectory();
void openLayoutDirectory();

} // namespace Paths

#endif // !PATHS_H

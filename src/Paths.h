#ifndef PATHS_H
#define PATHS_H

#include <filesystem>
#include <string_view>

namespace Paths {
std::filesystem::path getAppDataDir();
std::filesystem::path getActiveConfigPath();

std::filesystem::path appDataConfigsFolderPath();

/// @param name Config name without extension.
std::filesystem::path getConfigPath(std::string_view name);

/// @param name Config name without extension.
std::filesystem::path getPortableConfigPath(std::string_view name);

/// Returns the config file path depending on IS_PORTABLE constant
/// @param name Config name without extension.
/// @return Path to the `.cfg` file.
std::filesystem::path resolveConfigPath(std::string_view name);


bool ensureConfigDirectory();
} // namespace Paths

#endif // !PATHS_H

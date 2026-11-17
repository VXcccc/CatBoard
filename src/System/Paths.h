#pragma once

#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>

namespace Paths {

std::filesystem::path getAppDataDir();

std::filesystem::path getExecutableDir();
std::filesystem::path getAssetsDir();

std::filesystem::path getActiveConfigPath();
std::filesystem::path getUIJsonPath();

std::filesystem::path configDirectory();
std::filesystem::path layoutDirectory();
std::filesystem::path soundsDirectory();
std::vector<std::filesystem::path> fontsDirectories();
std::filesystem::path imagesDirectory();

std::filesystem::path getConfigPath(std::string_view name);
std::filesystem::path getLayoutPath(std::string_view name);

std::filesystem::path getSoundPackPath(std::string_view name);
std::filesystem::path getSoundPackJsonPath(std::string_view name);

std::optional<std::filesystem::path> getFontPath(const std::string& name);

std::filesystem::path getImagePackPath(std::string_view name);
std::filesystem::path getImagePackJson(std::string_view name);

bool ensureConfigDirectory();
bool ensureLayoutDirectory();
bool ensureSoundsDirectory();
// bool ensureFontsDirectory();
bool ensureImagesDirectory();

void openConfigDirectory();
void openLayoutDirectory();
void openSoundsDirectory();
void openFontsDirectory();
void openImagesDirectory();

void createAssetsDirectory();

} // namespace Paths

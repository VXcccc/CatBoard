#include "Paths.h"
#include "Constants.h"
#include <filesystem>
#include <shlobj.h>
#include <string_view>
#include <windows.h>

namespace Paths {
namespace {
std::filesystem::path categoryDirectory(std::string_view category) {
  if (Consts::IS_PORTABLE) {
    return std::filesystem::path{std::string{category}};
  }
  return getAppDataDir() / std::string{category};
}

std::filesystem::path categoryItemPath(std::string_view category,
                                       std::string_view name) {
  return categoryDirectory(category) / (std::string{name} + ".json");
}

bool ensureCategoryDirectory(std::string_view category) {
  const auto path = categoryDirectory(category);
  if (std::filesystem::exists(path))
    return true;
  return std::filesystem::create_directories(path);
}

void openCategoryDirectory(std::string_view category) {
  const auto path = categoryDirectory(category);
  ShellExecute(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}
} // namespace

std::filesystem::path getAppDataDir() {
  PWSTR path{nullptr};
  if (SUCCEEDED(
          SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path))) {
    std::filesystem::path result{std::filesystem::path{path} / "CatBoard"};
    CoTaskMemFree(path);
    return result;
  }
  return std::filesystem::current_path();
}

std::filesystem::path getExecutableDir() {
  return std::filesystem::current_path();
}

std::filesystem::path getActiveConfigPath() {
  return getAppDataDir() / "ActiveConfig.json";
}

std::filesystem::path getActiveLayoutPath() {
  return getAppDataDir() / "ActiveLayout.json";
}

std::filesystem::path configDirectory() { return categoryDirectory("Configs"); }
std::filesystem::path layoutDirectory() { return categoryDirectory("Layouts"); }
std::filesystem::path soundsDirectory() { return categoryDirectory("Sounds"); }

std::filesystem::path getConfigPath(std::string_view name) {
  return categoryItemPath("Configs", name);
}

std::filesystem::path getLayoutPath(std::string_view name) {
  return categoryItemPath("Layouts", name);
}

std::filesystem::path getSoundPackDir(std::string_view name) {
    return soundsDirectory() / std::string{name};
}

std::filesystem::path getSoundPackJsonPath(std::string_view name) {
    return getSoundPackDir(name) / "config.json";
}

bool ensureConfigDirectory() { return ensureCategoryDirectory("Configs"); }
bool ensureLayoutDirectory() { return ensureCategoryDirectory("Layouts"); }
bool ensureSoundsDirectory() { return ensureCategoryDirectory("Sounds"); }

void openConfigDirectory() { openCategoryDirectory("Configs"); }
void openLayoutDirectory() { openCategoryDirectory("Layouts"); }
void openSoundsDirectory() { openCategoryDirectory("Sounds"); }

} // namespace Paths

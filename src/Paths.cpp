#include "Paths.h"
#include "Constants.h"
#include <filesystem>
#include <shlobj.h>
#include <string_view>
#include <windows.h>

namespace Paths {
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

std::filesystem::path getActiveConfigPath() {
  return getAppDataDir() / "ActiveConfig.json";
}

std::filesystem::path appDataConfigsFolderPath() {
  return getAppDataDir() / "Configs";
}

std::filesystem::path getConfigPath(std::string_view name) {
  return appDataConfigsFolderPath() / (std::string{name} + ".json");
}

std::filesystem::path getPortableConfigPath(std::string_view name) {
  return std::filesystem::path{"Configs"} / (std::string{name} + ".json");
}

std::filesystem::path resolveConfigPath(std::string_view name) {
  if (Consts::IS_PORTABLE) {
    return getPortableConfigPath(name);
  }

  return getConfigPath(name);
}

std::filesystem::path configDirectory() {
  if (Consts::IS_PORTABLE) {
    return std::filesystem::path{"Configs"};
  }

  return appDataConfigsFolderPath();
}

bool ensureConfigDirectory() {
    const auto path = Paths::configDirectory();

    if (std::filesystem::exists(path)) {
        return true;
    }
    return std::filesystem::create_directories(path);
}

} // namespace Paths

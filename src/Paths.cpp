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

std::filesystem::path getExecutableDir() {
    return std::filesystem::current_path();
}

std::filesystem::path getActiveConfigPath() {
  return getAppDataDir() / "ActiveConfig.json";
}
  
std::filesystem::path getActiveLayoutPath() {
  return getAppDataDir() / "ActiveLayout.json";
}

std::filesystem::path appDataConfigsDirPath() {
  return getAppDataDir() / "Configs";
}

std::filesystem::path appDataLayoutsDirPath() {
  return getAppDataDir() / "Layouts";
}

std::filesystem::path getConfigPath(std::string_view name) {
  if (Consts::IS_PORTABLE) {
    return std::filesystem::path{"Configs"} / (std::string{name} + ".json");
  } 

  return appDataConfigsDirPath() / (std::string{name} + ".json");
}

std::filesystem::path getLayoutPath(std::string_view name) {
  if (Consts::IS_PORTABLE) {
    return std::filesystem::path{"Layouts"} / (std::string{name} + ".json");
  } 

  return appDataLayoutsDirPath() / (std::string{name} + ".json");
}

std::filesystem::path configDirectory() {
  if (Consts::IS_PORTABLE) {
    return std::filesystem::path{"Configs"};
  }

  return appDataConfigsDirPath();
}

std::filesystem::path layoutDirectory(){
  if(Consts::IS_PORTABLE){
    return std::filesystem::path{"Layouts"};
  }

  return appDataLayoutsDirPath();
}

bool ensureConfigDirectory() {
  const auto path = Paths::configDirectory();

  if (std::filesystem::exists(path)) {
    return true;
  }
  return std::filesystem::create_directories(path);
}

bool ensureLayoutDirectory(){
  const auto path = Paths::layoutDirectory();

  if (std::filesystem::exists(path)) {
    return true;
  }
  return std::filesystem::create_directories(path);
}

void openConfigDirectory() {
  const auto path{configDirectory()};
  ShellExecute(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

void openLayoutDirectory(){
  const auto path{layoutDirectory()};
  ShellExecute(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

} // namespace Paths

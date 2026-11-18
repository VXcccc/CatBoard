#include "Paths.h"
#include "Fonts.h"
#include <algorithm>
#include <array>
#include <filesystem>
#include <optional>
#include <shlobj.h>
#include <string_view>
#include <vector>
#include <windows.h>

namespace Paths {
namespace {
std::filesystem::path categoryDirectory(std::string_view category) {
  return getExecutableDir() / "Assets" / std::string{category};
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

static std::filesystem::path
getKnownFolder(const KNOWNFOLDERID &id,
               const std::filesystem::path &subDir = {}) {
  PWSTR path{nullptr};
  std::filesystem::path folderPath{};

  if (SUCCEEDED(SHGetKnownFolderPath(id, 0, NULL, &path))) {
    std::filesystem::path result{path};
    if (!subDir.empty()) {
      result /= subDir;
    }
    folderPath = result;
  }
  CoTaskMemFree(path);
  return folderPath;
}

std::filesystem::path getAppDataDir() {
  return getKnownFolder(FOLDERID_RoamingAppData, L"CatBoard");
}
std::filesystem::path getUserFontsFolder() {
  return getKnownFolder(FOLDERID_LocalAppData, L"Microsoft\\Windows\\Fonts");
}

std::filesystem::path getSystemFontsFolder() {
  return getKnownFolder(FOLDERID_Fonts);
}

std::vector<std::filesystem::path> fontsDirectories() {
  std::vector<std::filesystem::path> dirs{};
  dirs.push_back(getUserFontsFolder());
  dirs.push_back(getSystemFontsFolder());
  return dirs;
}

std::filesystem::path getExecutableDir() {
  static const std::filesystem::path dir = [] {
    wchar_t buf[32768]{};
    const DWORD n{
        GetModuleFileNameW(nullptr, buf, static_cast<DWORD>(std::size(buf)))};

    return n ? std::filesystem::path{buf}.parent_path()
             : std::filesystem::current_path();
  }();

  return dir;
}

std::filesystem::path getAssetsDir() { return getExecutableDir() / ("Assets"); }

std::filesystem::path getActiveConfigPath() {
  return getExecutableDir() / "ActiveConfig.json";
}

std::filesystem::path getUIJsonPath() { return getExecutableDir() / "UI.json"; }

std::filesystem::path configDirectory() { return categoryDirectory("Configs"); }
std::filesystem::path layoutDirectory() { return categoryDirectory("Layouts"); }
std::filesystem::path soundsDirectory() { return categoryDirectory("Sounds"); }
std::filesystem::path imagesDirectory() { return categoryDirectory("Images"); }

std::filesystem::path getConfigPath(std::string_view name) {
  return categoryItemPath("Configs", name);
}

std::filesystem::path getLayoutPath(std::string_view name) {
  return categoryItemPath("Layouts", name);
}

std::filesystem::path getSoundPackPath(std::string_view name) {
  return soundsDirectory() / std::string{name};
}

std::filesystem::path getSoundPackJsonPath(std::string_view name) {
  return getSoundPackPath(name) / "config.json";
}

static std::filesystem::path windowsFontsDir() {
  PWSTR raw = nullptr;
  std::filesystem::path result;

  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &raw))) {
    result = raw;
  }
  CoTaskMemFree(raw);

  return result;
}

std::optional<std::filesystem::path> getFontPath(const std::string &name) {
  const auto &it{Fonts::fontFiles.find(name)};
  if (it == Fonts::fontFiles.end())
    return std::nullopt;

  return it->second;
}

std::filesystem::path getImagePackPath(std::string_view name) {
  return imagesDirectory() / std::string{name};
}

std::filesystem::path getImagePackJson(std::string_view name) {
  return getImagePackPath(name) / "config.json";
}

bool ensureConfigDirectory() { return ensureCategoryDirectory("Configs"); }
bool ensureLayoutDirectory() { return ensureCategoryDirectory("Layouts"); }
bool ensureSoundsDirectory() { return ensureCategoryDirectory("Sounds"); }
// bool ensureFontsDirectory() { return ensureCategoryDirectory("Fonts"); }
bool ensureImagesDirectory() { return ensureCategoryDirectory("Images"); }

void openConfigDirectory() { openCategoryDirectory("Configs"); }
void openLayoutDirectory() { openCategoryDirectory("Layouts"); }
void openSoundsDirectory() { openCategoryDirectory("Sounds"); }
void openImagesDirectory() { openCategoryDirectory("Images"); }

void openFontsDirectory() {
  if (std::filesystem::exists(getSystemFontsFolder())) {
    ShellExecute(NULL, L"open", getSystemFontsFolder().c_str(), NULL, NULL,
                 SW_SHOWDEFAULT);
  }
}

static inline const std::array<std::string, 4> subDirs{"Configs", "Images",
                                                       "Layouts", "Sounds"};
void createAssetsDirectory() {
  const auto assetsDir{getAssetsDir()};

  std::filesystem::create_directories(assetsDir);
  for (const auto &sDir : subDirs) {
    std::filesystem::create_directories(assetsDir / sDir);
  }
}

} // namespace Paths

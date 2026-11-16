#include "Paths.h"
#include "Constants.h"
#include <filesystem>
#include <shlobj.h>
#include <string_view>
#include <windows.h>

namespace Paths {
namespace {
std::filesystem::path categoryDirectory(std::string_view category) {
  std::string portableDir{"Assets"};
  if (Consts::IS_PORTABLE) {
    return std::filesystem::path{portableDir} / std::string{category};
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
  static const std::filesystem::path dir = [] {
    wchar_t buf[32768]{};
    const DWORD n{
        GetModuleFileNameW(nullptr, buf, static_cast<DWORD>(std::size(buf)))};

    return n ? std::filesystem::path{buf}.parent_path()
             : std::filesystem::current_path();
  }();

  return dir;
}

std::filesystem::path getAssetsDir() { return getExecutableDir() / "Assets"; }

// ActiveConfig is always in appdata
std::filesystem::path getActiveConfigPath() {
  return getAppDataDir() / "ActiveConfig.json";
}

std::filesystem::path getUIJsonPath() { return getAppDataDir() / "UI.json"; }

std::filesystem::path configDirectory() { return categoryDirectory("Configs"); }
std::filesystem::path layoutDirectory() { return categoryDirectory("Layouts"); }
std::filesystem::path soundsDirectory() { return categoryDirectory("Sounds"); }
std::filesystem::path fontsDirectory() { return categoryDirectory("Fonts"); }
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

std::filesystem::path getFontPath(std::string_view name) {
  return fontsDirectory() / (std::string{name} + ".ttf");
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
bool ensureFontsDirectory() { return ensureCategoryDirectory("Fonts"); }
bool ensureImagesDirectory() { return ensureCategoryDirectory("Images"); }

void openConfigDirectory() { openCategoryDirectory("Configs"); }
void openLayoutDirectory() { openCategoryDirectory("Layouts"); }
void openSoundsDirectory() { openCategoryDirectory("Sounds"); }
void openFontsDirectory() { openCategoryDirectory("Fonts"); }
void openImagesDirectory() { openCategoryDirectory("Images"); }

} // namespace Paths

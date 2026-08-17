#include "imgui.h"
#include "Config.h"
#include <combaseapi.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <shlobj.h>
#include <strsafe.h>
#include <windows.h>

namespace Config {
Settings config{};

// maybe im gonna put this function in different header
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

static std::filesystem::path getConfigPath() {
  return getAppDataDir() / "config.json";
}

void Load() {
  auto path{getConfigPath()};

  // first load
  if (!std::filesystem::exists(path)) {
    Save(config);
    return;
  }

  try {
    std::ifstream file{path};
    config = json::parse(file).get<Settings>();
  } catch (const std::exception &) {
    config = Settings{};
  }
}

bool Save(const Settings &settings) {
  auto path = getConfigPath();
  std::filesystem::create_directories(path.parent_path());

  std::ofstream file{path};
  if (!file.is_open()) {
    return false;
  }

  file << json(settings).dump(3);
  config = settings;
  return file.good();
}

ImU32 ToImU32(const Config::Theme::ColorRGBA &c){
  return IM_COL32(c[0], c[1], c[2], c[3]);
}

} // namespace Config

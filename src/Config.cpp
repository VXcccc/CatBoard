#include "Config.h"
#include "ConfigJson.h"
#include "Constants.h"
#include "Paths.h"
#include "imgui.h"
#include <combaseapi.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <shlobj.h>
#include <string>
#include <string_view>
#include <strsafe.h>
#include <vector>
#include <windows.h>

namespace Config {
// current config
Settings activeConfig{};
static const Settings defaultConfig{};
std::vector<std::string> configs{};
std::string activeConfigName;

void refreshConfigs() {
  Config::configs.clear();
  const auto folder = Paths::configDirectory();

  if (!std::filesystem::exists(folder)) {
    return;
  }

  for (const auto &entry : std::filesystem::directory_iterator(folder)) {
    if (!entry.is_regular_file()) {
      continue;
    } else if (entry.path().extension() != ".json") {
      continue;
    }
    Config::configs.push_back(entry.path().stem().string());
  }
}

void Load() {
  const auto path{Paths::getActiveConfigPath()};

  // first load
  if (!std::filesystem::exists(path)) {
    Apply(activeConfig);
    return;
  }

  // silently reseting config to defaults
  try {
    std::ifstream file{path};
    const auto j = json::parse(file);
    activeConfig = j.get<Settings>();
  } catch (const std::exception &) {
    activeConfig = defaultConfig;
    Apply(activeConfig);
  }
}

bool Apply(const Settings &settings) {
  const auto path{Paths::getActiveConfigPath()};
  std::filesystem::create_directories(path.parent_path());

  std::ofstream file{path};
  if (!file) {
    return false;
  }

  file << json(settings).dump(4);
  activeConfig = settings;
  return file.good();
}

ImU32 ToImU32(const Config::Theme::ColorRGBA &c) {
  return IM_COL32(c[0], c[1], c[2], c[3]);
}

Config::CreateConfigResult createConfig(std::string_view name) {
  if (!Paths::ensureConfigDirectory()) {
    return CreateConfigResult::DirectoryError;
  }

  if (name.empty() || name.size() > 64)
    return CreateConfigResult::NameTooLong;

  const auto path{Paths::getConfigPath(name)};

  if (std::filesystem::exists(path)) {
    return CreateConfigResult::AlreadyExists;
  }

  std::ofstream file{path};
  if (!file) {
    return CreateConfigResult::FileError;
  }

  file << json(defaultConfig).dump(4);
  if (!file) {
    return CreateConfigResult::FileError;
  }

  return CreateConfigResult::Success;
}

bool saveConfig(std::string_view name, const Config::Settings &settings) {
  if (!Paths::ensureConfigDirectory()) {
    return false;
  }

  const auto path{Paths::getConfigPath(name)};

  std::ofstream file{path};

  if (!file) {
    return false;
  }
  file << json(settings).dump(4);

  return file.good();
}

bool loadConfig(std::string_view name) {
  const auto path{Paths::getConfigPath(name)};

  try {
    std::ifstream file{path};
    if (!file) {
      return false;
    }
    const auto j = json::parse(file);
    activeConfig = j.get<Settings>();
    activeConfigName = std::string{name};
  } catch (const std::exception &e) {
    // TODO: send message to imgui
    // notifyError{e.what()};
    return false;
  }
  return true;
}

bool deleteConfig(std::string_view name) {
  const auto path = Paths::getConfigPath(name);

  if (!std::filesystem::exists(path)) {
    return false;
  }

  return std::filesystem::remove(path);
}

} // namespace Config

#include "Config/Config.h"
#include "Config/ConfigJson.h"
#include "System/Helpers.h"
#include "System/Paths.h"
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

void refreshConfigs() {
  Helper::refresh(configs, Paths::configDirectory(), ".json");
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


Helper::FileCreateResult createConfig(std::string_view name) {
  if (!Paths::ensureConfigDirectory()) {
    return Helper::FileCreateResult::DirectoryError;
  }
  if (name.empty())
    return Helper::FileCreateResult::EmptyName;

  if (name.size() > 64)
    return Helper::FileCreateResult::NameTooLong;

  const auto path{Paths::getConfigPath(name)};

  if (std::filesystem::exists(path)) {
    return Helper::FileCreateResult::AlreadyExists;
  }

  std::ofstream file{path};
  if (!file) {
    return Helper::FileCreateResult::FileError;
  }

  file << json(defaultConfig).dump(4);
  if (!file) {
    return Helper::FileCreateResult::FileError;
  }

  return Helper::FileCreateResult::Success;
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
  } catch (const std::exception &e) {
    std::string err{e.what()};
    Helper::drawErrorInSettings(err);
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

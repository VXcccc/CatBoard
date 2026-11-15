#include "Config.h"
#include "Helpers.h"
#include "Hotkeys.h"
#include "JsonParser.h"
#include "Paths.h"
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
static const Settings defaultConfig{};
std::vector<std::string> configs{};

void refreshConfigs() {
  Helper::refresh(configs, Paths::configDirectory(), ".json");
}

void LoadActiveConfig(Settings &outConfig) {
  const auto path{Paths::getActiveConfigPath()};

  // first load
  if (!std::filesystem::exists(path)) {
    Apply(outConfig);
    return;
  }

  try {
    std::ifstream file{path};
    const auto j = json::parse(file);
    outConfig = j.get<Settings>();

		//checking for all hotkeys defined to be loaded
		outConfig.binds = Binds::syncBindsOrder(outConfig.binds);

		// silently reseting config to defaults(except name)
  } catch (const std::exception &) {
		Settings resultConfig {defaultConfig};
		resultConfig.configName = outConfig.configName;
    outConfig = resultConfig;
    Apply(outConfig);
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
  return file.good();
}

Helper::FileCreateResult Create(std::string_view name) {
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

	Settings resultConfig {defaultConfig};
	resultConfig.configName = name;
  file << json(resultConfig).dump(4);
  if (!file) {
    return Helper::FileCreateResult::FileError;
  }

  return Helper::FileCreateResult::Success;
}

bool Save(std::string_view name, const Config::Settings &settings) {
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

bool Load(std::string_view name, Settings &outConfig) {
  const auto path{Paths::getConfigPath(name)};

  try {
    std::ifstream file{path};
    if (!file) {
      return false;
    }
    const auto j = json::parse(file);
    outConfig = j.get<Settings>();
  } catch (const std::exception &e) {
    std::string err{e.what()};
    Helper::drawErrorInSettings(err);
    return false;
  }
  return true;
}

bool Delete(std::string_view name) {
  const auto path = Paths::getConfigPath(name);

  if (!std::filesystem::exists(path)) {
    return false;
  }

  return std::filesystem::remove(path);
}

} // namespace Config

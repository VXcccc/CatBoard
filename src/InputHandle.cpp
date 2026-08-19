#include "InputHandle.h"
#include "Paths.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <fstream>
#include <string_view>
#include <vector>

namespace Input {
std::vector<std::string> layouts{};
Layout activeLayout{};
std::string defaultLayoutName{"layout7t"};


void refreshLayouts() {
  layouts.clear();

  const auto folder = Paths::layoutDirectory();

  if (!std::filesystem::exists(folder))
    return;

  for (const auto &entry : std::filesystem::directory_iterator(folder)) {
    if (!entry.is_regular_file())
      continue;

    if (entry.path().extension() != ".json")
      continue;

    layouts.push_back(entry.path().stem().string());
  }
}

bool installDefaultLayouts() {
  const auto source{Paths::getExecutableDir() /
                    "Layouts"}; // TODO fix this function(cuz idk how its gonna
                                // work without layouts dir in executable dir)
  const auto destination{Paths::layoutDirectory()};

  if (std::filesystem::exists(destination)) {
    return true;
  }

  std::error_code ec;

  std::filesystem::copy(source, destination,
                        std::filesystem::copy_options::recursive, ec);

  return !ec;
}

bool loadLayoutFile(std::string_view name) {
  try {
    std::ifstream file{Paths::getLayoutPath(name)};

    if (!file)
      return false;

    activeLayout = json::parse(file).get<Layout>();
    return true;
  } catch (const std::exception &) {
    return false;
  }
}

bool loadLayout(std::string_view name) {
  if (loadLayoutFile(name))
    return true;

  if (name != defaultLayoutName)
    return loadLayoutFile(defaultLayoutName);

  return false;
}

} // namespace Input

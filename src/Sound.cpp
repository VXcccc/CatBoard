#include "Paths.h"

#include <Sound.h>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <windows.h>

namespace Sound {
std::vector<std::string> sounds;
PackVK activePack;
std::string defaultSoundPack{"cherrymx-red-abs"};

void refreshSounds() {
  sounds.clear();

  const auto folder = Paths::soundsDirectory();

  if (!std::filesystem::exists(folder))
    return;

  for (const auto &entry : std::filesystem::directory_iterator(folder)) {
    if (!entry.is_directory())
      continue;
    sounds.push_back(entry.path().filename().string());
  }
}

std::optional<PackVK> Load(const std::filesystem::path &packDir) {
  std::ifstream file{packDir / "config.json"};
  if (!file) {
    return std::nullopt;
  }

  try {
    Sound::Pack pack{json::parse(file).get<Pack>()};
    PackVK loaded;
    loaded.audioFile = (packDir / pack.audio_file).string();

    const auto &codeToVK = CodeToVK();
    for (const auto &[code, def] : pack.definitions) {
      auto it{codeToVK.find(code)};
      if (it != codeToVK.end()) {
        loaded.KeyDefByVK[it->second] = def;
      } else {
        loaded.unknownCodes.push_back(code);
      }
    }
    return loaded;
  } catch (const std::exception &) {
    return std::nullopt;
  }
}

bool loadSoundPack(std::string_view name) {
  auto packDir = Paths::getSoundPackDir(name);
  auto result = Load(packDir);
  if (!result) {
    return false;
  }
  activePack = std::move(*result);
  return true;
}

bool installDefaultSoundPacks() {
  const auto source{Paths::getExecutableDir() /
                    "Sounds"}; // TODO fix this function(cuz idk how its gonna
                               // work without layouts dir in executable dir)
  const auto destination{Paths::soundsDirectory()};

  if (std::filesystem::exists(destination)) {
    return true;
  }

  std::error_code ec;

  std::filesystem::copy(source, destination,
                        std::filesystem::copy_options::recursive, ec);

  return !ec;
}

const std::unordered_map<std::string, int> &CodeToVK() {
  static const std::unordered_map<std::string, int> map = {
      {"KeyA", 'A'},
      {"KeyB", 'B'},
      {"KeyC", 'C'},
      {"KeyD", 'D'},
      {"KeyE", 'E'},
      {"KeyF", 'F'},
      {"KeyG", 'G'},
      {"KeyH", 'H'},
      {"KeyI", 'I'},
      {"KeyJ", 'J'},
      {"KeyK", 'K'},
      {"KeyL", 'L'},
      {"KeyM", 'M'},
      {"KeyN", 'N'},
      {"KeyO", 'O'},
      {"KeyP", 'P'},
      {"KeyQ", 'Q'},
      {"KeyR", 'R'},
      {"KeyS", 'S'},
      {"KeyT", 'T'},
      {"KeyU", 'U'},
      {"KeyV", 'V'},
      {"KeyW", 'W'},
      {"KeyX", 'X'},
      {"KeyY", 'Y'},
      {"KeyZ", 'Z'},
      {"Digit0", '0'},
      {"Digit1", '1'},
      {"Digit2", '2'},
      {"Digit3", '3'},
      {"Digit4", '4'},
      {"Digit5", '5'},
      {"Digit6", '6'},
      {"Digit7", '7'},
      {"Digit8", '8'},
      {"Digit9", '9'},

      {"ShiftLeft", VK_LSHIFT},
      {"ShiftRight", VK_RSHIFT},
      {"ControlLeft", VK_LCONTROL},
      {"ControlRight", VK_RCONTROL},
      {"AltLeft", VK_LMENU},
      {"AltRight", VK_RMENU},
      {"CapsLock", VK_CAPITAL},
      {"Tab", VK_TAB},
      {"Enter", VK_RETURN},
      {"Backspace", VK_BACK},
      {"Space", VK_SPACE},
      {"Escape", VK_ESCAPE},
      {"Delete", VK_DELETE},
      {"Insert", VK_INSERT},
      {"Home", VK_HOME},
      {"End", VK_END},
      {"PageUp", VK_PRIOR},
      {"PageDown", VK_NEXT},
      {"ArrowUp", VK_UP},
      {"ArrowDown", VK_DOWN},
      {"ArrowLeft", VK_LEFT},
      {"ArrowRight", VK_RIGHT},
      {"PrintScreen", VK_SNAPSHOT},
      {"ScrollLock", VK_SCROLL},
      {"Pause", VK_PAUSE},
      {"NumLock", VK_NUMLOCK},
      {"F1", VK_F1},
      {"F2", VK_F2},
      {"F3", VK_F3},
      {"F4", VK_F4},
      {"F5", VK_F5},
      {"F6", VK_F6},
      {"F7", VK_F7},
      {"F8", VK_F8},
      {"F9", VK_F9},
      {"F10", VK_F10},
      {"F11", VK_F11},
      {"F12", VK_F12},

      {"Numpad0", VK_NUMPAD0},
      {"Numpad1", VK_NUMPAD1},
      {"Numpad2", VK_NUMPAD2},
      {"Numpad3", VK_NUMPAD3},
      {"Numpad4", VK_NUMPAD4},
      {"Numpad5", VK_NUMPAD5},
      {"Numpad6", VK_NUMPAD6},
      {"Numpad7", VK_NUMPAD7},
      {"Numpad8", VK_NUMPAD8},
      {"Numpad9", VK_NUMPAD9},
      {"NumpadAdd", VK_ADD},
      {"NumpadSubtract", VK_SUBTRACT},
      {"NumpadMultiply", VK_MULTIPLY},
      {"NumpadDivide", VK_DIVIDE},
      {"NumpadDecimal", VK_DECIMAL},

      {"Semicolon", VK_OEM_1},
      {"Equal", VK_OEM_PLUS},
      {"Comma", VK_OEM_COMMA},
      {"Minus", VK_OEM_MINUS},
      {"Period", VK_OEM_PERIOD},
      {"Slash", VK_OEM_2},
      {"Backquote", VK_OEM_3},
      {"BracketLeft", VK_OEM_4},
      {"Backslash", VK_OEM_5},
      {"BracketRight", VK_OEM_6},
      {"Quote", VK_OEM_7},
  };
  return map;
}

} // namespace Sound

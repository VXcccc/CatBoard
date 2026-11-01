#include "Helpers.h"
#include "Paths.h"

#include "Config.h"
#include "Constants.h"
#include "Sound.h"
#include "extras/decoders/libvorbis/miniaudio_libvorbis.h"
#include "miniaudio.h"
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <windows.h>

namespace Sound {
std::vector<std::string> sounds;
std::string defaultSoundPack{"cherrymx-red-abs"};

static ma_resource_manager g_resourceManager;
static ma_engine g_engine;
// static ma_sound g_activeSound;

static std::vector<ma_sound> g_voices;

// static bool g_soundLoaded = false;

ma_uint64 msToFrames(double ms) {
  return static_cast<ma_uint64>(
      ms * static_cast<double>(ma_engine_get_sample_rate(&g_engine)) / 1000.0);
}

bool initAudio() {
  static ma_decoding_backend_vtable *s_backends[] = {
      ma_decoding_backend_libvorbis,
  };

  ma_resource_manager_config cfg = ma_resource_manager_config_init();
  cfg.ppCustomDecodingBackendVTables = s_backends;
  cfg.customDecodingBackendCount =
      static_cast<ma_uint32>(std::size(s_backends));
  cfg.pCustomDecodingBackendUserData = nullptr;

  if (ma_resource_manager_init(&cfg, &g_resourceManager) != MA_SUCCESS) {
    return false;
  }

  ma_engine_config engineCfg = ma_engine_config_init();
  engineCfg.pResourceManager = &g_resourceManager;

  if (ma_engine_init(&engineCfg, &g_engine) != MA_SUCCESS) {
    ma_resource_manager_uninit(&g_resourceManager);
    return false;
  }

  return true;
}

void ShutdownAudio() {
  ma_engine_uninit(&g_engine);
  ma_resource_manager_uninit(&g_resourceManager);
}

void refreshSounds() {
  Helper::refresh(sounds, Paths::soundsDirectory(), "dir");
}

std::optional<PackVK> Load(const std::filesystem::path &packDir) {
  std::ifstream file{packDir / "config.json"};
  if (!file) {
    return std::nullopt;
  }

  try {
    Sound::Pack pack{json::parse(file).get<Pack>()};
    PackVK loaded;
    loaded.name = pack.name;
    loaded.audio_file = pack.audio_file;

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

bool initVoicePool(const std::filesystem::path &audioPath) {
  g_voices.resize(Consts::VOICE_COUNT);

  for (auto &voice : g_voices) {
    const ma_result result{ma_sound_init_from_file(
        &g_engine, audioPath.string().c_str(), MA_SOUND_FLAG_DECODE, nullptr,
        nullptr, &voice)};

    if (result != MA_SUCCESS) {
      return false;
    }
  }
  return true;
}

void uninitVoicePool() {
  for (auto &voice : g_voices) {
    ma_sound_uninit(&voice);
  }
  g_voices.clear();
}

ma_sound *findFreeVoice() {
  for (auto &voice : g_voices) {
    if (!ma_sound_is_playing(&voice)) {
      return &voice;
    }
  }
  return nullptr; // maybe todo system that gonna stole the resource for voice
}

bool loadSoundPack(std::string_view name) {
  auto packDir = Paths::getSoundPackPath(name);
  auto result = Load(packDir);

  if (!result) {
    return false;
  }

  uninitVoicePool();
  activePack = std::move(*result);

  const auto audioPath{Paths::getSoundPackPath(name) / activePack.audio_file};

  return initVoicePool(audioPath);
}

void playKey(int keyCode, bool is_down) {
  auto it{activePack.KeyDefByVK.find(keyCode)};
  if (it == activePack.KeyDefByVK.end()) {
    return;
  }

  const KeyDef &keyDef{it->second};

  const auto &down{keyDef.keyDown()};
  const auto &up{keyDef.keyUp()};

  double startms{};
  double endms{};

  if (is_down) {
    startms = down[0];
    endms = down[1];
  } else {
    startms = up[0];
    endms = up[1];
  }

  ma_sound *voice{findFreeVoice()};
  if (!voice) {
    return;
  }

  const ma_uint64 lengthFrames{msToFrames(endms) - msToFrames(startms)};
  const ma_uint64 now = ma_engine_get_time_in_pcm_frames(&g_engine);

  ma_sound_set_start_time_in_pcm_frames(voice, now);
  ma_sound_set_stop_time_in_pcm_frames(voice, now + lengthFrames);

  ma_sound_seek_to_pcm_frame(voice, msToFrames(startms));
  ma_sound_start(voice);
}

void setMasterVolume(float volume) { ma_engine_set_volume(&g_engine, volume); }

bool installDefaultSoundPacks() {
  return Helper::installDefault(Paths::getExecutableDir() / "Sounds",
                                Paths::soundsDirectory());
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

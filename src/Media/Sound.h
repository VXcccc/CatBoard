// shoutout mechvibes https://github.com/hainguyents13/mechvibes
#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Sound {

using TimingRange = std::array<double, 2>; // startms //endms

struct KeyDef {
  std::array<TimingRange, 2> timing;

	bool hasRelease{false};

  const TimingRange &keyDown() const { return timing[0]; }

  const TimingRange &keyUp() const { return timing[1]; }
};

// json parsed pack
struct Pack {
  std::string name;
  std::string audio_file; // named with underscore like in json
  std::unordered_map<std::string, KeyDef> definitions;
};

// pack translated to VK codes used in AppState
struct PackVK {
  std::string name;
  std::string audio_file;
  std::unordered_map<int, KeyDef> KeyDefByVK;

	// debug
  std::vector<std::string> unknownCodes;
};

void refreshSounds();
bool loadSoundPack(std::string_view name, PackVK &outPack);

bool initAudio();
void ShutdownAudio();

void playKey(int keyCode, bool is_down, const PackVK &pack);
void setMasterVolume(float volume);

// std::optional<PackVK> Load(const std::filesystem::path &path);
const std::unordered_map<std::string, int> &CodeToVK();

extern std::vector<std::string> sounds;
extern const std::string defaultSoundPack;
} // namespace Sound

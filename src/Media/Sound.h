// shoutout mechvibes
#ifndef SOUND_H
#define SOUND_H

#include <array>
#include <filesystem>
#include <optional>
#include <string_view>
#include "nlohmann/json.hpp"
#include <string>
#include <unordered_map>
#include <vector>
using json = nlohmann::json;

namespace Sound {

using TimingRange = std::array<double, 2>; // startms //endms

struct KeyDef {
  std::array<TimingRange, 2> timing;

  const TimingRange &keyDown() const { return timing[0]; }

  const TimingRange &keyUp() const { return timing[1]; }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KeyDef, timing)

struct Pack {
  std::string name;
  std::string audio_file;
  std::unordered_map<std::string, KeyDef> definitions;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Pack, name, audio_file, definitions)

struct PackVK {
  std::string name;
  std::string audio_file;
  std::unordered_map<int, KeyDef> KeyDefByVK;
  std::vector<std::string> unknownCodes;
};

void refreshSounds();
bool loadSoundPack(std::string_view name, PackVK &outPack);
bool installDefaultSoundPacks();

bool initAudio();
void ShutdownAudio();


void playKey(int keyCode, bool is_down, const PackVK &pack);
void setMasterVolume(float volume);

std::optional<PackVK> Load(const std::filesystem::path &path);
const std::unordered_map<std::string, int> &CodeToVK();

extern std::vector<std::string> sounds;
extern const std::string defaultSoundPack;
} // namespace Sound

#endif // !SOUND_H

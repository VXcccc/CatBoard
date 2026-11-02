#pragma once
#include "Constants.h"
#include "imgui.h"
#include <bitset>
#include <vector>

// forward declarations
namespace Layouts {
struct Layout;
}
namespace Mouse {
class State;
}
namespace Config {
struct Theme;
struct Settings;
}
namespace Sound {
struct PackVK;
}
struct AppState;

namespace Keyboard {
using KeyBitSet = std::bitset<Consts::VK_COUNT>;
struct KeyColors {
  ImVec4 fill{};
  ImVec4 text{};
  ImVec4 outline{};
};

struct KeyVisual {
  bool active{false};
  float scale{1.0f};
  KeyColors colors{};
};

struct State {
  KeyBitSet down{};
  KeyBitSet prevDown{};
  std::vector<KeyVisual> visuals{};

  void pollFullKeyboard();
  bool anyChanged() const { return (down ^ prevDown).any(); }
  bool pressed(int vk) const { return down[vk] && !prevDown[vk]; }
  bool released(int vk) const { return !down[vk] && prevDown[vk]; }
  bool inRange(int vk) const { return vk > 0 && vk < Consts::VK_COUNT; }

  // visuals vector is synced with layout indicies
  void resyncVisuals(const Layouts::Layout &layout,
                     const Config::Theme &theme);
};

// bool detectAnyMouseKeyChanged(Keyboard::State &state);

// void handleKeySound(const Layouts::Layout &layout,
//                     const Keyboard::State &state);
//
// void handleKeySoundFull(const Keyboard::State &state);

void hadnleKeySounds(const Keyboard::State &kb, bool layoutKeysOnly,
                     const Layouts::Layout &layout,
                     const Sound::PackVK &soundPack);

void updateActive(const Layouts::Layout &layout, Keyboard::State &state,
                  const Mouse::State &mouse,
                  const Config::Settings &settings);

bool updateKeyColors(const Layouts::Layout &layout, Keyboard::State &state,
                     float deltaTime, const Config::Theme &theme);

bool updateScale(const Layouts::Layout &layout, Keyboard::State &state,
                 float deltaTime, const Config::Theme &theme);

void renderKeyboard( AppState &app);
} // namespace Keyboard

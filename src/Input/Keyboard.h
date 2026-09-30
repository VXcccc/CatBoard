#pragma once
#include <array>

// forward declarations
namespace Layouts {
struct Layout;
}
namespace Render {
struct KeyColors;
}

namespace Keyboard {
bool detectAnyKeyChanged(const Layouts::Layout &layout,
                         const std::array<bool, 256> &prevKeyState,
                         std::array<bool, 256> &outCurrentDown);

bool detectAnyMouseKeyChanged(const std::array<bool, 256> &prevKeyState,
                              std::array<bool, 256> &outCurrentDown);

void commitKeyState(const Layouts::Layout &layout,
                    const std::array<bool, 256> &currentKeysDown,
                    std::array<bool, 256> &outPrevKeyState);

void handleKeySound(const Layouts::Layout &layout,
                    const std::array<bool, 256> &prevKeyState,
                    const std::array<bool, 256> &currentDown);

void handleKeySoundFull(const std::array<bool, 256> &prevKeyState,
                        const std::array<bool, 256> &currentKeysDown);

void pollFullKeyboard(std::array<bool, 256> &outCurrentKeysDown);

bool updateKeyColors(const Layouts::Layout &layout,
                     const std::array<bool, 256> &currentKeysDown,
                     std::array<Render::KeyColors, 256> &outColors,
                     float deltaTime);

bool updateAnimations(const Layouts::Layout &layout,
                      const std::array<bool, 256> &currentKeysDown,
                      std::array<float, 256> &outKeysScale, float deltaTime);

void renderKeyboard(const Layouts::Layout &layout,
                    const std::array<bool, 256> &currentKeysDown,
                    const std::array<bool, 256> &currentFullKeysDown,
                    const std::array<float, 256> &keysScale,
                    const std::array<Render::KeyColors, 256> &keysColors);
} // namespace Keyboard

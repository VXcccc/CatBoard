#pragma once

#include "Config/Constants.h"
#include "UI/Layouts.h"
#include "imgui.h"
#include <bitset>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <windows.h>

// forward declarations
namespace Keyboard {
using KeyBitSet = std::bitset<Consts::VK_COUNT>;
}

namespace Icons{
	constexpr char arrowUp{'n'};
	constexpr char arrowDown{'k'};
}

namespace Fonts {
extern const std::string defaultFontName;
extern const float defaultPixelsSize;

extern ImFont *overlayFont;
extern ImFont *iconFont;

extern std::vector<std::string> fonts;

enum class LetterCaseMode { defaultMode, allCapital, allLower, maxLetterCase };

struct LetterCaseOptions {
  bool capsInsensitiveKeys{false};
  bool capsSensitiveKeys{false};
  bool capsLockDisplay{false};
};

struct LetterCase {
  LetterCaseMode mode{LetterCaseMode::defaultMode};
  LetterCaseOptions options{};
  bool upper{false};
};

struct Font {
  std::string name{defaultFontName};
  float sizePixels{defaultPixelsSize};
  LetterCase letterCase{};
};

bool loadFont(const Font &font, ImFont *&activeFont);

// void updateFont(Font &outLastAppliedFont, ImFont *&activeFont,
//                 float explicitSize);
void updateFont(Font &outLastAppliedFont, ImFont *&activeFont);

bool installDefaultFonts();
void refreshFonts();

std::string applyLetterCase(const std::string &label, LetterCaseMode mode);

std::string handleLetterCase(const Keyboard::KeyBitSet &currentFullKeysDown,
                             Font &font, const Layouts::Key &key);

} // namespace Fonts

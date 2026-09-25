#pragma once

#include "UI/Layouts.h"
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <windows.h>

namespace Fonts {
extern const std::string defaultFontName;
extern const float defaultPixelsSize;

extern ImFont *activeFont;
extern std::vector<std::string> fonts;

enum class LetterCaseMode { defaultMode, allCapital, allLower, maxLetterCase };

struct LetterCaseOptions {
  bool capsInsensitiveKeys{false};
  bool capsSensitiveKeys{false};
  bool capsLockDisplay{false};
};

struct LetterCase {
  LetterCaseMode mode{LetterCaseMode::allLower};
  LetterCaseOptions options{};
  bool upper{false};
};

struct Font {
  std::string name{defaultFontName};
  float sizePixels{defaultPixelsSize};
  LetterCase letterCase{};
};

bool loadFont(const Font &font);
bool installDefaultFonts();
void refreshFonts();

std::string applyLetterCase(const std::string &label, LetterCaseMode mode);

std::string handleLetterCase(const std::array<bool, 256> &currentFullKeysDown,
                             Font &font, const Layouts::Key &key);

} // namespace Fonts

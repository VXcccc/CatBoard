#pragma once

#include "imgui.h"
#include "InputHandle.h"
#include "nlohmann/detail/macro_scope.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <windows.h>
#include <string_view>
#include <utility>
#include <vector>

namespace Fonts {
extern const std::string defaultFontName;
extern const float defaultPixelsSize;

extern ImFont *activeFont;
extern std::vector<std::string> fonts;

enum class LetterCaseMode { defaultMode , allCapital, allLower, maxLetterCase };

struct LetterCaseOptions{
  bool capsInsensitiveKeys{false};
  bool capsSensitiveKeys{false};
  bool capsLockDisplay{false};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LetterCaseOptions, capsInsensitiveKeys, capsSensitiveKeys, capsLockDisplay)

struct LetterCase{
  LetterCaseMode mode{LetterCaseMode::allLower};
  LetterCaseOptions options{};
  bool upper{false};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LetterCase, mode, options)

struct Font {
  std::string name{defaultFontName};
  float sizePixels{defaultPixelsSize};
  LetterCase letterCase{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Font, name, sizePixels, letterCase)

bool loadFont(const Font &font);
bool installDefaultFonts();
void refreshFonts();

std::string applyLetterCase(const std::string &label, LetterCaseMode mode);


std::string handleLetterCase(const std::array<bool, 256> &currentFullKeysDown,
                             Font &font, const Input::Key &key);



} // namespace Fonts

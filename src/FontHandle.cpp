#include "FontHandle.h"
#include "Helpers.h"
#include "Paths.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <winuser.h>

namespace Fonts {
std::vector<std::string> fonts{};
ImFont *activeFont{nullptr};
const std::string defaultFontName{"courbd"};
const float defaultPixelsSize{20.0f};

bool loadFont(const Font &font) {
  ImGuiIO &io{ImGui::GetIO()};

  io.Fonts->Clear();

  std::string pathString{Paths::getFontPath(font.name).string()};
  activeFont =
      io.Fonts->AddFontFromFileTTF(pathString.c_str(), font.sizePixels);

  if (!activeFont) {
    return false;
  }

  io.Fonts->Build();

  ImGui_ImplDX11_InvalidateDeviceObjects();
  ImGui_ImplDX11_CreateDeviceObjects();

  return true;
}

bool installDefaultFonts() {
  const auto source{Paths::getExecutableDir() /
                    "Fonts"}; // TODO fix this function(cuz idk how its gonna
                              // work without fonts dir in executable dir)
  const auto destination{Paths::fontsDirectory()};

  if (std::filesystem::exists(destination)) {
    return true;
  }

  std::error_code ec;

  std::filesystem::copy(source, destination,
                        std::filesystem::copy_options::recursive, ec);

  return !ec;
}

bool isLetterKey(const Layouts::Key &key) {
  return (key.keyCode >= 'A' && key.keyCode <= 'Z');
}

std::string changeStringCase(std::string_view label, bool upper) {
  auto lambda{upper ? [](unsigned char c) { return std::toupper(c); }
                    : [](unsigned char c) { return std::tolower(c); }};

  std::string result{label};
  std::transform(result.begin(), result.end(), result.begin(), lambda);

  return result;
}

std::string applyLetterCase(const std::string &label, Font &font) {

  switch (font.letterCase.mode) {
  case LetterCaseMode::allCapital: {
    font.letterCase.upper = true;
    return changeStringCase(label, font.letterCase.upper);
  }
  case LetterCaseMode::allLower: {
    font.letterCase.upper = false;
    return changeStringCase(label, font.letterCase.upper);
  }
  case LetterCaseMode::defaultMode: {
    font.letterCase.upper = false;
    return label;
  }
  default:
    return label;
  }
}

std::string handleCapsLock(const std::array<bool, 256> &currentFullKeysDown,
                           Font &font, const Layouts::Key &key,
                           const bool capsLockDown) {
  font.letterCase.upper = !font.letterCase.upper;
  if (capsLockDown) {
    if (isLetterKey(key)) {
      return changeStringCase(key.label, font.letterCase.upper);
    }
  }
  return applyLetterCase(key.label, font);
}

std::string handleShift(const std::array<bool, 256> &currentFullKeysDown,
                        Font &font, const Layouts::Key &key,
                        const bool shiftDown) {
  static const std::unordered_map<int, char> vkToShiftSymbol{
      {'1', '!'}, {'2', '@'},  {'3', '#'}, {'4', '$'}, {'5', '%'},
      {'6', '^'}, {'7', '&'},  {'8', '*'}, {'9', '('}, {'0', ')'},
      {'-', '_'}, {'=', '+'},  {'[', '{'}, {']', '}'}, {'\\', '|'},
      {';', ':'}, {'\'', '"'}, {',', '<'}, {'.', '>'}, {'/', '?'}};

  font.letterCase.upper = !font.letterCase.upper;

  if (shiftDown) {
    if (font.letterCase.options.capsInsensitiveKeys) {
      auto it{vkToShiftSymbol.find(key.keyCode)};
      if (it != vkToShiftSymbol.end()) {
        return {it->second};
      }
    }

    if (font.letterCase.options.capsSensitiveKeys) {
      if (isLetterKey(key)) {
        return changeStringCase(key.label, font.letterCase.upper);
      }
    }
  }

  return applyLetterCase(key.label, font);
}

std::string handleLetterCase(const std::array<bool, 256> &currentFullKeysDown,
                             Font &font, const Layouts::Key &key) {
  bool capsLockDown{(GetKeyState(VK_CAPITAL) & 1) != 0};
  bool shiftDown{currentFullKeysDown[VK_LSHIFT] ||
                 currentFullKeysDown[VK_RSHIFT]};

  std::string result{applyLetterCase(key.label, font)};

  if (font.letterCase.options.capsLockDisplay) {
    if (capsLockDown) {
      result = handleCapsLock(currentFullKeysDown, font, key, capsLockDown);
    }
  }

  if (font.letterCase.options.capsInsensitiveKeys ||
      font.letterCase.options.capsSensitiveKeys) {
    if (shiftDown) {
      result = handleShift(currentFullKeysDown, font, key, shiftDown);
    }
  }

  return result;
}

void refreshFonts() { Helper::refresh(fonts, Paths::fontsDirectory(), ".ttf"); }

} // namespace Fonts

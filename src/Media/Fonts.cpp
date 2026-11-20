
#include "Fonts.h"
#include <optional>
#include <unordered_map>
#define STB_TRUETYPE_IMPLEMENTATION
#include "Config.h"
#include "Constants.h"
#include "Helpers.h"
#include "Keyboard.h"
#include "Paths.h"
#include "Resource/PreInstalledFonts.h"
#include "imgui.h"
#include "imstb_truetype.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include <winuser.h>

namespace Fonts {
std::vector<std::string> fonts{};
std::unordered_map<std::string, std::filesystem::path> fontFiles{};


ImFont *settingsFont{nullptr};
ImFont *overlayFont{nullptr};
ImFont *iconFont{nullptr};
const std::string defaultFontName{"courbd"};
const float defaultPixelsSize{20.0f};

static bool canImGuiLoad(const std::filesystem::path &path) {
  std::ifstream file{path, std::ios::binary};
  if (!file)
    return false;

  std::vector<unsigned char> data{(std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>()};
  if (data.empty())
    return false;

  const int offset{stbtt_GetFontOffsetForIndex(data.data(), 0)};
  stbtt_fontinfo info;
  return offset >= 0 && stbtt_InitFont(&info, data.data(), offset) != 0;
}

bool canLoad(const std::string &name) {
  if (const auto path{Paths::getFontPath(name)}) {
    return std::filesystem::exists(path.value()) && canImGuiLoad(path.value());
  };
  return false;
}

bool loadIconFont(float overlayFontSize) {
  ImGuiIO &io{ImGui::GetIO()};

  ImFontConfig cfg;
  cfg.PixelSnapH = true;
  cfg.FontDataOwnedByAtlas = false;
  iconFont = io.Fonts->AddFontFromMemoryTTF(_dripIcons, _dripIconsSize,
                                            overlayFontSize, &cfg);

  if (!iconFont)
    return false;

  return true;
}

bool loadFont(const Font &font, ImFont *&activeFont) {
  ImGuiIO &io{ImGui::GetIO()};
  std::filesystem::path path{};
  if (Paths::getFontPath(font.name)) {
    path = Paths::getFontPath(font.name).value();
  } else {
    return false;
  }
  if (!std::filesystem::exists(path) || !canImGuiLoad(path)) {
    return false;
  }

  io.Fonts->ClearFonts();
	ImFontConfig cfg;
  activeFont =
      io.Fonts->AddFontFromFileTTF(path.string().c_str(), font.sizePixels);
  if (!activeFont) {
    return false;
  }

  if (!loadIconFont(font.sizePixels)) {
    Helper::errorMesBox(L"Failed to Load icon font");
    return false;
  }

  return true;
}

void updateFont(Font &outLastAppliedFont, ImFont *&activeFont, Font &font) {
  if (font.name != outLastAppliedFont.name ||
      font.sizePixels != outLastAppliedFont.sizePixels) {
    if (!loadFont(font, activeFont)) {
      const std::wstring failedName{Helper::strToWide(font.name)};
      font.name = defaultFontName;
      font.sizePixels = defaultPixelsSize;
      loadFont(Fonts::Font{}, activeFont);
      // Helper::errorMesBox((L"Failed to load font \"" + failedName +
      //                      L"\".\nReset to default font.").c_str());
    }
    outLastAppliedFont = font;
  }
}

// bool isLetterKey(const Layouts::Key &key) {
//   return (key.code >= 'A' && key.code <= 'Z');
// }

bool isLetterKey(int vkCode) {
  HWND foreground{GetForegroundWindow()};
  DWORD threadId{GetWindowThreadProcessId(foreground, nullptr)};
  HKL layout{GetKeyboardLayout(threadId)};

  BYTE keyState[256]{};
  WCHAR buffer[5]{};
  UINT scanCode{MapVirtualKeyExW(vkCode, MAPVK_VK_TO_VSC, layout)};

  int result{ToUnicodeEx(vkCode, scanCode, keyState, buffer, 5, 0, layout)};
  return result > 0 && iswalpha(buffer[0]);
}

// std::string changeStringCase(std::string_view label, bool upper) {
//   auto lambda{upper ? [](unsigned char c) { return std::toupper(c); }
//                     : [](unsigned char c) { return std::tolower(c); }};
//
//   std::string result{label};
//   std::transform(result.begin(), result.end(), result.begin(), lambda);
//
//   return result;
// }

std::string changeStringCase(std::string_view label, bool upper) {
  int wlen{MultiByteToWideChar(CP_UTF8, 0, label.data(), (int)label.size(),
                               nullptr, 0)};
  std::wstring wide(wlen, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, label.data(), (int)label.size(), wide.data(),
                      wlen);

  if (upper) {
    CharUpperBuffW(wide.data(), (DWORD)wide.size());
  } else {
    CharLowerBuffW(wide.data(), (DWORD)wide.size());
  }

  int u8len{WideCharToMultiByte(CP_UTF8, 0, wide.data(), (int)wide.size(),
                                nullptr, 0, nullptr, nullptr)};
  std::string result(u8len, '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), (int)wide.size(), result.data(),
                      u8len, nullptr, nullptr);
  return result;
}

std::string applyLetterCase(const std::string &label, Font &font) {

  switch (font.letterCase.mode) {
  case LetterCaseMode::AllCapital: {
    font.letterCase.upper = true;
    return changeStringCase(label, font.letterCase.upper);
  }
  case LetterCaseMode::AllLower: {
    font.letterCase.upper = false;
    return changeStringCase(label, font.letterCase.upper);
  }
  case LetterCaseMode::DefaultMode: {
    font.letterCase.upper = false;
    return label;
  }
  default:
    return label;
  }
}

std::string handleCapsLock(const Keyboard::KeyBitSet &currentFullKeysDown,
                           Font &font, const Layouts::Key &key,
                           const bool capsLockDown) {
  font.letterCase.upper = !font.letterCase.upper;
  if (capsLockDown) {
    if (isLetterKey(key.code)) {
      return changeStringCase(key.label, font.letterCase.upper);
    }
  }
  return applyLetterCase(key.label, font);
}

std::string handleShift(const Keyboard::KeyBitSet &currentFullKeysDown,
                        Font &font, const Layouts::Key &key,
                        const bool shiftDown) {

  font.letterCase.upper = !font.letterCase.upper;

  if (shiftDown) {
    if (font.letterCase.options.capsInsensitiveKeys) {
      if (key.shiftLabel == Consts::BLANK_LABEL) {
        return {""};
      }
      if (!key.shiftLabel.empty()) {
        return {key.shiftLabel};
      }
    }

    if (font.letterCase.options.capsSensitiveKeys) {
      if (isLetterKey(key.code)) {
        return changeStringCase(key.label, font.letterCase.upper);
      }
    }
  }

  return applyLetterCase(key.label, font);
}

std::string handleLetterCase(const Keyboard::KeyBitSet &keysDown, Font &font,
                             const Layouts::Key &key) {
  bool capsLockDown{(GetKeyState(VK_CAPITAL) & 1) != 0};
  bool shiftDown{keysDown[VK_LSHIFT] || keysDown[VK_RSHIFT]};

  std::string result{applyLetterCase(key.label, font)};

  if (font.letterCase.options.capsLockDisplay) {
    if (capsLockDown) {
      result = handleCapsLock(keysDown, font, key, capsLockDown);
    }
  }

  if (font.letterCase.options.capsInsensitiveKeys ||
      font.letterCase.options.capsSensitiveKeys) {
    if (shiftDown) {
      result = handleShift(keysDown, font, key, shiftDown);
    }
  }

  return result;
}

bool refreshFonts() {
  fonts.clear();

  for (const auto &dir : Paths::fontsDirectories()) {
    if (!std::filesystem::exists(dir))
      continue;

    std::error_code ec{};

    for (const auto &entry : std::filesystem::directory_iterator(dir, ec)) {
      if (!entry.is_regular_file(ec))
        continue;

      std::string ext{entry.path().extension().string()};
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
      if (ext != ".ttf" && ext != ".otf")
        continue;
      fontFiles[entry.path().stem().string()] = entry.path();
    }
  }

  if (std::empty(fontFiles))
    return false;

  for (const auto &[name, path] : fontFiles) {
    fonts.push_back(name);
    std::sort(fonts.begin(), fonts.end());
  }

  return true;
}

} // namespace Fonts

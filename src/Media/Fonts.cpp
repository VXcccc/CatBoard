#include "Media/Fonts.h"
#include "Config/Constants.h"
#include "System/Helpers.h"
#include "System/Paths.h"
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

  const auto path{Paths::getFontPath(font.name)};

  if (!std::filesystem::exists(path)) {
    return false;
  }

  io.Fonts->Clear();

  activeFont =
      io.Fonts->AddFontFromFileTTF(path.string().c_str(), font.sizePixels);

  if (!activeFont) {
    return false;
  }

  io.Fonts->Build();

  ImGui_ImplDX11_InvalidateDeviceObjects();
  ImGui_ImplDX11_CreateDeviceObjects();

  return true;
}

bool installDefaultFonts() {
  return Helper::installDefault(Paths::getExecutableDir() / "Fonts",
                                Paths::fontsDirectory());
}

// bool isLetterKey(const Layouts::Key &key) {
//   return (key.keyCode >= 'A' && key.keyCode <= 'Z');
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
    if (isLetterKey(key.keyCode)) {
      return changeStringCase(key.label, font.letterCase.upper);
    }
  }
  return applyLetterCase(key.label, font);
}

std::string handleShift(const std::array<bool, 256> &currentFullKeysDown,
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
      if (isLetterKey(key.keyCode)) {
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

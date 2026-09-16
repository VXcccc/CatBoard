#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>
#include <windows.h>
#include "imgui.h"

#define IDI_ICON1 101

namespace Consts {
constexpr int CATBOARD_VERSION {1};
constexpr bool IS_PORTABLE {false};

constexpr UINT WM_TRAYICON{WM_APP + 1};

constexpr UINT ID_ABOUT     {1002};
constexpr UINT ID_EXIT      {1003};
constexpr UINT ID_DRAGMODE  {1004};
constexpr UINT ID_SETTINGS  {1005};

constexpr std::size_t VOICE_COUNT {32};

constexpr ImVec4 defaultClearColor{0.0f, 0.0f, 0.0f, 0.0f};
}

#endif // !CONSTANTS_H

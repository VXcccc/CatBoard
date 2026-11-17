#pragma once

#include "imgui.h"
#include <cstddef>
#include <string>
#include <windows.h>

#ifndef WS_EX_NOREDIRECTIONBITMAP
#define WS_EX_NOREDIRECTIONBITMAP 0x00200000L
#endif

namespace Consts {
constexpr int CONFIG_VERSION{1};
constexpr int CATBOARD_VERSION{APP_VERSION_INT};
constexpr std::string CATBOARD_VERSION_STR{APP_VERSION_STR};


constexpr UINT WM_TRAYICON{WM_APP + 1};

constexpr UINT ID_ABOUT{1002};
constexpr UINT ID_EXIT{1003};
constexpr UINT ID_DRAGMODE{1004};
constexpr UINT ID_SETTINGS{1005};

constexpr std::size_t VOICE_COUNT{32};

constexpr ImVec4 defaultClearColor{0.0f, 0.0f, 0.0f, 0.0f};

constexpr std::size_t MAX_UNDO_STEPS{100};

constexpr double ERROR_DELETE_TIME{5.0};

constexpr std::string BLANK_LABEL{"BLANK_LABEL"};

constexpr float EDITOR_MINZOOM{1.0f};
constexpr float EDITOR_MAXZOOM{100.0f};

constexpr int VK_COUNT{256};

constexpr std::string EMPTY_HOTKEY_NAME{"None"};

} // namespace Consts


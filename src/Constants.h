#include <windows.h>
#ifndef CONSTANTS_H
#define CONSTANTS_H


#define IDI_ICON1 101

namespace Consts {
constexpr int CATBOARD_VERSION {1};

constexpr UINT WM_TRAYICON{WM_APP + 1};

constexpr UINT ID_ABOUT     {1002};
constexpr UINT ID_EXIT      {1003};
constexpr UINT ID_DRAGMODE  {1004};
constexpr UINT ID_SETTINGS  {1005};
}

#endif // !CONSTANTS_H

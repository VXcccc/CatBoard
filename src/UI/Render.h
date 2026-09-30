#pragma once

// to do add backends abstraction for cross platform use
#include "imgui.h"
namespace Render {
struct KeyColors {
  ImVec4 fill;
  ImVec4 text;
  ImVec4 outline;
};
} // namespace Render

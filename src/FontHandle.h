#pragma once

#include <string_view>
#include <string>
#include <vector>
#include "imgui.h"
#include "nlohmann/detail/macro_scope.hpp"
#include <nlohmann/json.hpp>

namespace Fonts {
  extern const std::string defaultFontName;
  extern const float defaultPixelsSize;

  extern ImFont *activeFont;
  extern std::vector<std::string> fonts;

  struct Font{
    std::string name{defaultFontName};
    float sizePixels{defaultPixelsSize};
  };
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Font, name, sizePixels)

  bool loadFont(const Font& font);
  bool installDefaultFonts();
  void refreshFonts();

}

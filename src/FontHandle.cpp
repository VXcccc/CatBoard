#include "FontHandle.h"
#include "Paths.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <string>
#include <vector>

namespace Fonts {
std::vector<std::string> fonts{};
ImFont *activeFont{nullptr};
const std::string defaultFontName{"courbd"};
const float defaultPixelsSize{20.0f};

bool loadFont(const Font &font) {
    ImGuiIO &io {ImGui::GetIO()};

    io.Fonts->Clear();

    std::string pathString {Paths::getFontPath(font.name).string()};
    activeFont = io.Fonts->AddFontFromFileTTF(pathString.c_str(), font.sizePixels);

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

void refreshFonts() {
  fonts.clear();

  const auto folder {Paths::fontsDirectory()};

  if (!std::filesystem::exists(folder))
    return;

  for (const auto &entry : std::filesystem::directory_iterator(folder)) {
    if (!entry.is_regular_file())
      continue;

    if (entry.path().extension() != ".ttf")
      continue;

    fonts.push_back(entry.path().stem().string());
  }
}

} // namespace Fonts

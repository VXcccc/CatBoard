#pragma once

#include "imgui.h"
#include "nlohmann/detail/macro_scope.hpp"
#include <d3d11.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
using json = nlohmann::json;

namespace Images {
struct Image {
  std::string backGroundFileName{"keyb"};
  std::string activeFileName{"keyw"};
  int vkCode{0};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Image, backGroundFileName, activeFileName,
                                   vkCode)
struct MouseButton {
  std::string buttonIdle{};
  std::string buttonActive{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MouseButton, buttonIdle, buttonActive)

struct Mouse {
  MouseButton left{"leftb", "leftbp"};
  MouseButton right{"rightb", "rightbp"};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Mouse, left, right)

struct ImagePack {
  std::string name{"DefaultImagePack"};
  Image defaultKey{};
  std::vector<Image> explicitImages{};
  Mouse mouse{};
  bool enable{false};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ImagePack, name, explicitImages, defaultKey,
                                   mouse, enable)

struct Texture {
  ID3D11ShaderResourceView *srv{};
  int w{};
  int h{};
};

bool installDefaultImagePacks();
bool LoadTextureFromFile(const char *file_name,
                         ID3D11ShaderResourceView **out_srv, int *out_width,
                         int *out_height);

void drawKeyboardImages(ImDrawList *draw,
                        const std::array<bool, 256> &currentKeysDown);
bool handleConfigJson(std::string_view name);
void drawMouseImages(ImDrawList *draw, const ImVec2 &center,
                     const std::array<bool, 256> &currentKeysDown);

}; // namespace Images

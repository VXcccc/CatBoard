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
  float x{0.0f};
  float y{0.0f};
  float w{0.0f};
  float h{0.0f};
  std::string buttonIdle{};
  std::string buttonActive{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MouseButton, buttonIdle, buttonActive, x, y, w, h)

struct Mouse {
  MouseButton left{-31.0f, 0.0f, 63.0f, 219.0f, "leftb", "leftbp"};
  MouseButton right{31.5f, 0.0f, 62.0f, 219.0f, "rightb", "rightbp"};
  MouseButton middle{0.5f, -67.5f, 12.0f, 38.0f, "midb", "midbp"};
  MouseButton scrollUp{0.5f, -77.0f, 12.0f, 19.0f, "sub", "suw"};
  MouseButton scrollDown{0.5f, -58.0f, 12.0f, 19.0f, "sdb", "sdw"};
  MouseButton mouse4{-51.0f, 5.0f, 5.0f, 41.0f, "m4b", "m4w"};
  MouseButton mouse5{-53.0f, -31.0f, 7.0f, 35.0f, "m5b", "m5w"};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Mouse, left, right, middle, scrollUp,
                                   scrollDown, mouse4, mouse5)

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
                     const std::array<bool, 256> &currentKeysDown,
                     bool scrollUpActive, bool scrollDownActive);

}; // namespace Images

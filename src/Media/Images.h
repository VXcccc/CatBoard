#pragma once

#include "Constants.h"
#include "imgui.h"
#include <array>
#include <bitset>
#include <d3d11.h>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
using json = nlohmann::json;
// forward declarations
namespace Keyboard {
struct State;
using KeyBitSet = std::bitset<Consts::VK_COUNT>;
} // namespace Keyboard

namespace Config {
struct Theme;
}

struct AppState;
namespace Layouts {
struct Layout;
}

namespace Images {
struct ImageOpacity {
  float mouse{};
  float keys{};
  bool updateImageOpacity(const Config::Theme &theme, float deltaTime,
                          bool hideOverlay, int hideAnimSpeed);
};

struct Image {
  std::string backGroundFileName{"keyb"};
  std::string activeFileName{"keyw"};
  int vkCode{0};
  bool operator==(const Image &) const = default;
};

// enum class MBType { button, scrollDown, scrollUp, max_MBTypes };
struct MouseButton {
  float x{0.0f};
  float y{0.0f};
  float w{0.0f};
  float h{0.0f};
  std::string buttonIdle{};
  std::string buttonActive{};
  // MBType type{MBType::button};
};
bool operator==(const MouseButton &mb1, const MouseButton &mb2);
bool operator!=(const MouseButton &mb1, const MouseButton &mb2);

struct Mouse {
  MouseButton left{-31.0f, 0.0f, 63.0f, 219.0f, "leftb", "leftbp"};
  MouseButton right{31.5f, 0.0f, 62.0f, 219.0f, "rightb", "rightbp"};
  MouseButton middle{0.5f, -67.5f, 12.0f, 38.0f, "sb", "sw"};
  MouseButton scrollUp{0.5f, -77.0f, 12.0f, 19.0f, "sub", "suw"};
  MouseButton scrollDown{0.5f, -58.0f, 12.0f, 19.0f, "sdb", "sdw"};
  MouseButton mouse4{-51.0f, 5.0f, 5.0f, 41.0f, "m4b", "m4w"};
  MouseButton mouse5{-53.0f, -31.0f, 7.0f, 35.0f, "m5b", "m5w"};
  MouseButton bottom{};

};

struct Pack {
  std::string name{"DefaultImagePack"};
  Image defaultKey{};
  std::vector<Image> explicitImages{};
  Mouse mouse{};

  ImageOpacity opacity{};
};


struct Texture {
  ID3D11ShaderResourceView *srv{};
  int w{};
  int h{};
};

// bool LoadTextureFromFile(const char *file_name,
//                          ID3D11ShaderResourceView **out_srv, int *out_width,
//                          int *out_height);

void drawKeyboardImages(ImDrawList *draw, const AppState &app);

bool Load(std::string_view name, Pack &outPack);
void drawMouseImages(ImDrawList *draw, const ImVec2 &center,
                     const AppState &app);

void refreshImagePacks();

void drawSizeOfImages(const Pack &pack, const Layouts::Layout &layout);

bool isMouseKeysHeld(const AppState &app);

extern const Images::Pack defaultPack;
extern std::vector<std::string> packs;
}; // namespace Images

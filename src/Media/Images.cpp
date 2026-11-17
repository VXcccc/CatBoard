#include "Images.h"
#include "AppState.h"
#include "Config.h"
#include "Fonts.h"
#include "Helpers.h"
#include "JsonParser.h"
#include "Keyboard.h"
#include "Layouts.h"
#include "MainWindow.h"
#include "Mouse.h"
#include "Paths.h"
#include "imgui.h"
#include <algorithm>
#include <array>
#include <conio.h>
#include <cstdint>
#include <cstdlib>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <format>
#include <fstream>
#include <hidclass.h>
#include <hidusage.h>
#include <shellapi.h>
#include <string>
#include <tchar.h>
#include <windef.h>
#include <windows.h>
#include <winnt.h>
#include <winuser.h>

#include <strsafe.h>

#include <mmsystem.h>

#include <dcomp.h>

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Images {
// extern
const Images::Pack defaultPack{};
std::vector<std::string> packs{};

// static
static std::unordered_map<std::string, Texture> g_textureCache;


bool operator==(const MouseButton &mb1, const MouseButton &mb2) {
  return (mb1.x == mb2.x && mb1.y == mb2.y && mb1.w == mb2.w &&
          mb1.h == mb2.h && mb1.buttonIdle == mb2.buttonIdle &&
          mb1.buttonActive == mb2.buttonActive);
}
bool operator!=(const MouseButton &mb1, const MouseButton &mb2) {
  return !(mb1 == mb2);
}

void refreshImagePacks() {
  Helper::refresh(packs, Paths::imagesDirectory(), "dir");
};

// Simple helper function to load an image into a DX11 texture with common
// settings
bool LoadTextureFromMemory(const void *data, size_t data_size,
                           ID3D11ShaderResourceView **out_srv, int *out_width,
                           int *out_height) {
  // Load from disk into a raw RGBA buffer
  int image_width = 0;
  int image_height = 0;
  unsigned char *image_data =
      stbi_load_from_memory((const unsigned char *)data, (int)data_size,
                            &image_width, &image_height, NULL, 4);
  if (image_data == NULL)
    return false;

  // Create texture
  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width = image_width;
  desc.Height = image_height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;

  ID3D11Texture2D *pTexture = NULL;
  D3D11_SUBRESOURCE_DATA subResource;
  subResource.pSysMem = image_data;
  subResource.SysMemPitch = desc.Width * 4;
  subResource.SysMemSlicePitch = 0;
  MainWindow::get_g_pd3dDevice()->CreateTexture2D(&desc, &subResource,
                                                  &pTexture);

  // Create texture view
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
  ZeroMemory(&srvDesc, sizeof(srvDesc));
  srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = desc.MipLevels;
  srvDesc.Texture2D.MostDetailedMip = 0;
  MainWindow::get_g_pd3dDevice()->CreateShaderResourceView(pTexture, &srvDesc,
                                                           out_srv);
  pTexture->Release();

  *out_width = image_width;
  *out_height = image_height;
  stbi_image_free(image_data);

  return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
bool LoadTextureFromFile(const char *file_name,
                         ID3D11ShaderResourceView **out_srv, int *out_width,
                         int *out_height) {
  FILE *f = fopen(file_name, "rb");
  if (f == NULL)
    return false;
  fseek(f, 0, SEEK_END);
  size_t file_size = (size_t)ftell(f);
  if (file_size == -1)
    return false;
  fseek(f, 0, SEEK_SET);
  void *file_data = IM_ALLOC(file_size);
  fread(file_data, 1, file_size, f);
  fclose(f);
  bool ret = LoadTextureFromMemory(file_data, file_size, out_srv, out_width,
                                   out_height);
  IM_FREE(file_data);
  return ret;
}

Texture getOrLoadTexture(const std::string &fileName, const Pack &pack) {
  const auto path{
      (Paths::getImagePackPath(pack.name) / (fileName + ".png")).string()};
  auto it{g_textureCache.find(path)};
  if (it != g_textureCache.end()) {
    return it->second;
  }

  ID3D11ShaderResourceView *srv{nullptr};
  int w{0};
  int h{0};
  if (LoadTextureFromFile(path.c_str(), &srv, &w, &h)) {
    g_textureCache[path] = Texture{srv, w, h};
    return Texture{srv, w, h};
  }
  return Texture{nullptr};
}

void clearTextureCache() {
  for (auto &[name, texture] : g_textureCache) {
    if (texture.srv)
      texture.srv->Release();
  }
  g_textureCache.clear();
}

bool Load(std::string_view name, Pack &outPack) {
  if (!Paths::ensureImagesDirectory()) {
    return false;
  }
  clearTextureCache();

  const auto path{Paths::getImagePackJson(name)};

  try {
    std::ifstream file{path};
    if (!file) {
      return false;
    }
    const auto j = json::parse(file);
    outPack = j.get<Images::Pack>();
  } catch (const std::exception &e) {
    std::string err{e.what()};
    // Helper::drawErrorInSettings(err);
    return false;
  }
  return true;
}

const Image *findImageForKey(int vkCode, const Pack &pack) {
  auto it =
      std::find_if(pack.explicitImages.begin(), pack.explicitImages.end(),
                   [&](const Image &image) { return image.vkCode == vkCode; });
  if (it != pack.explicitImages.end()) {
    return &*it;
  }
  return &pack.defaultKey;
}

void drawSizeOfImages(const Pack &pack, const Layouts::Layout &layout) {
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.0f, 0.00f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.00f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.0f, 0.0f, 0.0f, 0.00f));
  const bool open{ImGui::CollapsingHeader("Images Size")};
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();

  if (!open)
    return;

  std::string output{};
  bool defaultRecorded{false};
  for (const auto &key : layout.keys) {
    const Image *image{findImageForKey(key.code, pack)};
    // maybe by activeFileName is better idk
    const std::string &fileName{image->backGroundFileName};
    Texture texture{getOrLoadTexture(fileName, pack)};
    if (!texture.srv)
      continue;

    if (*image == pack.defaultKey && !defaultRecorded) {
      output.append(
          std::format("Default key: W: {}, H: {} \n", texture.w, texture.h));
      defaultRecorded = true;
    } else if (*image != pack.defaultKey) {
      output.append(std::format("{}: W: {}, H: {} \n",
                                image->backGroundFileName, texture.w,
                                texture.h));
    }
  }

  ImGui::TextWrapped("%s", output.c_str());
}

void drawKeyboardImages(ImDrawList *draw, const AppState &app) {
  for (std::size_t i{0}; i < app.layout.keys.size(); ++i) {
    const auto &v{app.keyboard.visuals[i]};
    const auto &key{app.layout.keys[i]};

    bool down{v.active};

    const Image *image{findImageForKey(key.code, app.imagePack)};
    const std::string &fileName{down ? image->activeFileName
                                     : image->backGroundFileName};

    Texture texture{getOrLoadTexture(fileName, app.imagePack)};
    if (!texture.srv)
      continue;

    const auto &ovScale{app.config.overlay.scale};
    ImVec2 topLeft{key.x * ovScale, key.y * ovScale};
    // todo fix the problem with different size of key in layout and in image
    // size
    ImVec2 bottomRight{(key.x + key.w) * ovScale, (key.y + key.h) * ovScale};
    draw->AddImage((ImTextureID)(intptr_t)texture.srv, topLeft, bottomRight,
                   ImVec2(0, 0), ImVec2(1, 1),
                   IM_COL32(255, 255, 255, app.config.theme.keysImageOpacity));
  }
}

void handleScroll(ImDrawList *draw, const MouseButton &scroll,
                  const ImVec2 &partCenter, bool up,
                  const ::Mouse::State &mouseState,
                  const Config::MouseTheme &mouseTheme) {
  ImGui::PushFont(Fonts::overlayFont);
  int counter{up ? mouseState.getScrollUpTimes()
                 : mouseState.getScrollDownTimes()};
  if (counter > 0) {
    std::string label{std::to_string(counter)};

    ImVec2 textSize{ImGui::CalcTextSize(label.c_str())};
    ImVec2 textPos{partCenter.x - (textSize.x * 0.5f),
                   partCenter.y - (textSize.y * 0.5f)};
    ImU32 textColor{Helper::ToImU32(mouseTheme.scrollCounter)};
    draw->AddText(textPos, textColor, label.c_str());
  }
  ImGui::PopFont();
}

void drawMousePart(ImDrawList *draw, const ImVec2 &center,
                   const MouseButton &part, bool active, const Pack &pack,
                   const AppState &app) {
  const std::string &fileName{active ? part.buttonActive : part.buttonIdle};
  if (fileName.empty())
    return;
  Texture texture{getOrLoadTexture(fileName, pack)};
  if (!texture.srv)
    return;

  const auto &mouse{app.config.theme.mouse};

  const auto &ovScale{app.config.overlay.scale};

  float partX{((part.x + mouse.imageOffsetX) * mouse.imageScale) * ovScale};
  float partY{((part.y + mouse.imageOffsetY) * mouse.imageScale) * ovScale};
  ImVec2 partCenter{center.x + partX, center.y + partY};

  ImVec2 topLeft{
      (partCenter.x - ((part.w * mouse.imageScale) * ovScale) * 0.5f),
      partCenter.y - ((part.h * mouse.imageScale) * ovScale) * 0.5f};
  ImVec2 bottomRight{topLeft.x + ((part.w * mouse.imageScale) * ovScale),
                     topLeft.y + ((part.h * mouse.imageScale) * ovScale)};

  draw->AddImage((ImTextureID)(intptr_t)texture.srv, topLeft, bottomRight,
                 ImVec2(0, 0), ImVec2(1, 1),
                 IM_COL32(255, 255, 255, app.config.theme.mouse.imageOpacity));
  if (part == pack.mouse.scrollUp) {
    handleScroll(draw, part, partCenter, true, app.mouse, mouse);
  } else if (part == pack.mouse.scrollDown) {
    handleScroll(draw, part, partCenter, false, app.mouse, mouse);
  }
}

void drawMouseImages(ImDrawList *draw, const ImVec2 &center,
                     const AppState &app) {
  const Mouse &mouse{app.imagePack.mouse};
  const auto &mouseState{app.mouse};
  const auto &keysDown{app.keyboard.down};

  drawMousePart(draw, center, mouse.left, keysDown[VK_LBUTTON], app.imagePack,
                app);
  drawMousePart(draw, center, mouse.right, keysDown[VK_RBUTTON], app.imagePack,
                app);
  drawMousePart(draw, center, mouse.mouse4, keysDown[VK_XBUTTON1],
                app.imagePack, app);
  drawMousePart(draw, center, mouse.mouse5, keysDown[VK_XBUTTON2],
                app.imagePack, app);
  drawMousePart(draw, center, mouse.bottom, false, app.imagePack, app);
  // fix middle

  drawMousePart(draw, center, mouse.middle, keysDown[VK_MBUTTON], app.imagePack,
                app);

  drawMousePart(draw, center, mouse.scrollUp, mouseState.isScrollingUp(),
                app.imagePack, app);
  drawMousePart(draw, center, mouse.scrollDown, mouseState.isScrollingDown(),
                app.imagePack, app);
}
} // namespace Images

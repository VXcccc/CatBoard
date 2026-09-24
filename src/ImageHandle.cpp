#include "ImageHandle.h"
#include "Constants.h"
#include "Helpers.h"
#include "Layouts.h"
#include "MainWindow.h"
#include "Paths.h"
#include "imgui.h"
#include <InputHandle.h>
#include <algorithm>
#include <array>
#include <conio.h>
#include <cstdint>
#include <cstdlib>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <fstream>
#include <hidclass.h>
#include <hidusage.h>
#include <shellapi.h>
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
#include "third_party/stb_image.h"

namespace Images {
// extern
ImagePack activeImagePack{};

// static
static std::unordered_map<std::string, Texture> g_textureCache;

bool installDefaultImagePacks() {
  return Helper::installDefault(Paths::getExecutableDir() / "Images",
                                Paths::imagesDirectory());
}

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

Texture getOrLoadTexture(const std::string &fileName) {
  auto it{g_textureCache.find(fileName)};
  if (it != g_textureCache.end()) {
    return it->second;
  }

  ID3D11ShaderResourceView *srv{nullptr};
  int w{0};
  int h{0};
  auto path{
      (Paths::getImagePackPath(activeImagePack.name) / (fileName + ".png"))
          .string()};

  if (LoadTextureFromFile(path.c_str(), &srv, &w, &h)) {
    g_textureCache[fileName] = Texture{srv, w, h};
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

// debug func
bool handleConfigJson(std::string_view name) {
  if (!Paths::ensureImagesDirectory()) {
    return false;
  }

  const auto path{Paths::getImagePackJson(name)};

  try {
    std::ifstream file{path};
    if (!file) {
      return false;
    }
    const auto j = json::parse(file);
    activeImagePack = j.get<ImagePack>();
  } catch (const std::exception &e) {
    std::string err{e.what()};
    // Helper::drawErrorInSettings(err);
    return false;
  }
  return true;
}

const Image *findImageForKey(int vkCode) {
  auto it =
      std::find_if(activeImagePack.explicitImages.begin(),
                   activeImagePack.explicitImages.end(),
                   [&](const Image &image) { return image.vkCode == vkCode; });
  if (it != activeImagePack.explicitImages.end()) {
    return &*it;
  }
  return &activeImagePack.defaultKey;
}

void drawKeyboardImages(ImDrawList *draw,
                        const std::array<bool, 256> &currentKeysDown) {
  for (const auto &key : Layouts::activeLayout.keys) {
    bool down{currentKeysDown[key.keyCode]};
    const Image *image{findImageForKey(key.keyCode)};
    const std::string &fileName{down ? image->activeFileName
                                     : image->backGroundFileName};

    Texture texture{getOrLoadTexture(fileName)};
    if (!texture.srv)
      continue;

    ImVec2 topLeft{key.x, key.y};
    //todo fix the problem with different size of key in layout and in image size 
    ImVec2 bottomRight{key.x + key.w, key.y + key.h};
    draw->AddImage((ImTextureID)(intptr_t)texture.srv, topLeft, bottomRight);
  }
}

void drawMouseImages(ImDrawList *draw, const ImVec2 &center,
                     const std::array<bool, 256> &currentKeysDown) {
  bool lDown{currentKeysDown[VK_LBUTTON]};
  bool rDown{currentKeysDown[VK_RBUTTON]};
  const Images::Mouse mouse{activeImagePack.mouse};

  const std::string &lFileName{lDown ? mouse.left.buttonActive
                                     : mouse.left.buttonIdle};
  const std::string &rFileName{rDown ? mouse.right.buttonActive
                                     : mouse.right.buttonIdle};

  Texture lTexture{getOrLoadTexture(lFileName)};
  Texture rTexture{getOrLoadTexture(rFileName)};

  const float totalWidth{static_cast<float>(lTexture.w + rTexture.w)};

  const float leftX{center.x - totalWidth * 0.5f};

  ImVec2 lTopLeft{leftX, center.y - lTexture.h * 0.75f};
  ImVec2 lBottomRight{lTopLeft.x + lTexture.w,
                      lTopLeft.y + lTexture.h};

  ImVec2 rTopLeft{lBottomRight.x, center.y - rTexture.h * 0.75f};
  ImVec2 rBottomRight{rTopLeft.x + rTexture.w,
                      rTopLeft.y + rTexture.h};
  if (lTexture.srv) {
    draw->AddImage((ImTextureID)(intptr_t)lTexture.srv, lTopLeft, lBottomRight);
  }
  if (rTexture.srv) {
    draw->AddImage((ImTextureID)(intptr_t)rTexture.srv, rTopLeft, rBottomRight);
  }
}

} // namespace Images

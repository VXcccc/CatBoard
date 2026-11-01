#pragma once

#include "Config.h"
#include "Keyboard.h"
#include "Sound.h"
#include "Images.h"
#include "Layouts.h"
#include "Mouse.h"

struct AppState {
  Keyboard::State keyboard{};
  Config::Settings config{};
  Layouts::Layout layout{};
  Images::Pack imagePack{};
  Sound::PackVK soundPack{};
  Mouse::MouseState mouse{config.theme.mouse.sensitivity,
                          config.theme.mouse.decayRate,
                          config.theme.mouse.fadeTime};
};

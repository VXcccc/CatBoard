#pragma once

#include "Config.h"
#include "Keyboard.h"
#include "Sound.h"
#include "Images.h"
#include "Layouts.h"
#include "Mouse.h"
#include "Themes.h"

struct AppState {
  Keyboard::State keyboard{};
  Config::Settings config{};
  Layouts::Layout layout{};
  Images::Pack imagePack{};
  Sound::PackVK soundPack{};
  Mouse::State mouse{};

	// bool resizeMode{false};
  bool dragMode{false};
  bool bindJustRecorded{false};
  bool previousObsMode{false};
  int uiTheme{Themes::defaultTheme};
};

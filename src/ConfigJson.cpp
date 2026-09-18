#include "ConfigJson.h"
#include "InputHandle.h"
#include <nlohmann/json.hpp>

namespace Config {

void to_json(json &j, const Outline &outline) {
  j = {{"enable", outline.enable},
       {"color", outline.color},
       {"thickness", outline.thickness}};
}

void from_json(const json &j, Outline &outline) {
  if (j.contains("enable"))
    j.at("enable").get_to(outline.enable);

  if (j.contains("color"))
    j.at("color").get_to(outline.color);

  if (j.contains("thickness"))
    j.at("thickness").get_to(outline.thickness);
}

void to_json(json &j, const Theme &theme) {
  j = {{"idleFill", theme.idleFill},
       {"activeFill", theme.activeFill},
       {"idleText", theme.idleText},
       {"activeText", theme.activeText},
       {"keyRounding", theme.keyRounding},
       {"keyOutline", theme.keyOutline},
       {"obsBackground", theme.obsBackground},

       {"mouseStyle", theme.mouseStyle},
       {"innerCircle", theme.innerCircle},
       {"outerCircle", theme.outerCircle},
       {"lerpColor", theme.lerpColor},
       {"radius", theme.radius},
       {"smallRadius", theme.smallRadius},
       {"outerThickness", theme.outerThickness}};
}

void from_json(const json &j, Theme &theme) {
  if (j.contains("idleFill"))
    j.at("idleFill").get_to(theme.idleFill);

  if (j.contains("activeFill"))
    j.at("activeFill").get_to(theme.activeFill);

  if (j.contains("idleText"))
    j.at("idleText").get_to(theme.idleText);

  if (j.contains("activeText"))
    j.at("activeText").get_to(theme.activeText);

  if (j.contains("keyRounding"))
    j.at("keyRounding").get_to(theme.keyRounding);

  if (j.contains("keyOutline"))
    j.at("keyOutline").get_to(theme.keyOutline);

  if (j.contains("obsBackground"))
    j.at("obsBackground").get_to(theme.obsBackground);

  if (j.contains("mouseStyle"))
    j.at("mouseStyle").get_to(theme.mouseStyle);

  if (j.contains("innerCircle"))
    j.at("innerCircle").get_to(theme.innerCircle);

  if (j.contains("outerCircle"))
    j.at("outerCircle").get_to(theme.outerCircle);

  if (j.contains("lerpColor"))
    j.at("lerpColor").get_to(theme.lerpColor);

  if (j.contains("radius"))
    j.at("radius").get_to(theme.radius);

  if (j.contains("smallRadius"))
    j.at("smallRadius").get_to(theme.smallRadius);

  if (j.contains("outerThickness"))
    j.at("outerThickness").get_to(theme.outerThickness);
}

void to_json(json &j, const SoundPackSelection &soundPack) {
  j = {{"name", soundPack.name},
       {"enable", soundPack.enable},
       {"layoutKeysOnlySound", soundPack.layoutKeysOnlySound},
       {"volume", soundPack.volume}};
}

void from_json(const json &j, SoundPackSelection &soundPack) {
  if (j.contains("name"))
    j.at("name").get_to(soundPack.name);
  if (j.contains("enable"))
    j.at("enable").get_to(soundPack.enable);
  if (j.contains("layoutKeysOnlySound"))
    j.at("layoutKeysOnlySound").get_to(soundPack.layoutKeysOnlySound);
  if (j.contains("volume"))
    j.at("volume").get_to(soundPack.volume);
}

void to_json(json &j, const Settings &settings) {
  j = {{"version", settings.version},
       {"theme", settings.theme},
       {"selectedLayout", settings.selectedLayout},
       {"soundPack", settings.soundPack},
       {"binds", settings.binds},
       {"overlayPos", settings.overlayPos},
       {"font", settings.font},
       {"misc", settings.misc},
       {"configName", settings.configName}};
}

void from_json(const json &j, Settings &settings) {
  if (j.contains("version"))
    j.at("version").get_to(settings.version);

  if (j.contains("theme"))
    j.at("theme").get_to(settings.theme);

  if (j.contains("selectedLayout"))
    j.at("selectedLayout").get_to(settings.selectedLayout);

  if (j.contains("soundPack"))
    j.at("soundPack").get_to(settings.soundPack);

  if (j.contains("binds"))
    j.at("binds").get_to(settings.binds);

  if (j.contains("overlayPos"))
    j.at("overlayPos").get_to(settings.overlayPos);

  if (j.contains("font"))
    j.at("font").get_to(settings.font);

  if (j.contains("misc"))
    j.at("misc").get_to(settings.misc);

  if (j.contains("configName"))
    j.at("configName").get_to(settings.configName);
}

void to_json(json &j, const Misc &misc) {
  j = {{"vsync", misc.vsync}, {"obsMode", misc.obsMode}, {"fps", misc.fps}};
}

void from_json(const json &j, Misc &misc) {

  if (j.contains("vsync"))
    j.at("vsync").get_to(misc.vsync);

  if (j.contains("obsMode"))
    j.at("obsMode").get_to(misc.obsMode);

  if (j.contains("fps"))
    j.at("fps").get_to(misc.fps);
}

void to_json(json &j, const OverlayPosition &overlayPos) {
  j = {{"x", overlayPos.x}, {"y", overlayPos.y}};
}

void from_json(const json &j, OverlayPosition &overlayPos) {
  if (j.contains("x"))
    j.at("x").get_to(overlayPos.x);
  if (j.contains("y"))
    j.at("y").get_to(overlayPos.y);
}

} // namespace Config

namespace Fonts {

void to_json(json &j, const LetterCaseOptions &options) {
  j = {{"capsInsensitiveKeys", options.capsInsensitiveKeys},
       {"capsSensitiveKeys", options.capsSensitiveKeys},
       {"capsLockDisplay", options.capsLockDisplay}};
}

void from_json(const json &j, LetterCaseOptions &options) {
  if (j.contains("capsInsensitiveKeys"))
    j.at("capsInsensitiveKeys").get_to(options.capsInsensitiveKeys);

  if (j.contains("capsSensitiveKeys"))
    j.at("capsSensitiveKeys").get_to(options.capsSensitiveKeys);

  if (j.contains("capsLockDisplay"))
    j.at("capsLockDisplay").get_to(options.capsLockDisplay);
}

void to_json(json &j, const LetterCase &letterCase) {
  j = {{"mode", letterCase.mode}, {"options", letterCase.options}};
}

void from_json(const json &j, LetterCase &letterCase) {
  if (j.contains("mode"))
    j.at("mode").get_to(letterCase.mode);

  if (j.contains("options"))
    j.at("options").get_to(letterCase.options);
}

void to_json(json &j, const Font &font) {
  j = {{"name", font.name},
       {"sizePixels", font.sizePixels},
       {"letterCase", font.letterCase}};
}

void from_json(const json &j, Font &font) {
  if (j.contains("name"))
    j.at("name").get_to(font.name);

  if (j.contains("sizePixels"))
    j.at("sizePixels").get_to(font.sizePixels);

  if (j.contains("letterCase"))
    j.at("letterCase").get_to(font.letterCase);
}
} // namespace Fonts

namespace Mouse {
void to_json(json &j, const Type &type) { j = static_cast<int>(type); }

void from_json(const json &j, Type &type) {
  type = static_cast<Type>(j.get<int>());
}

void to_json(json &j, const MouseStyle &mouseStyle) {
  j = {{"name", mouseStyle.name}, {"type", mouseStyle.type}};
}

void from_json(const json &j, MouseStyle &mouseStyle) {
  if (j.contains("name"))
    j.at("name").get_to(mouseStyle.name);

  if (j.contains("type"))
    j.at("type").get_to(mouseStyle.type);
}
} // namespace Mouse

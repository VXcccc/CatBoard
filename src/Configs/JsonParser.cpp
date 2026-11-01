#include "JsonParser.h"
#include "Config.h"
#include <nlohmann/json.hpp>

namespace Config {

void to_json(json &j, const Outline &outline) {
  j = {{"enable", outline.enable},
       {"idleColor", outline.idleColor},
       {"activeColor", outline.activeColor},
       {"thickness", outline.thickness}};
}

void from_json(const json &j, Outline &outline) {
  if (j.contains("enable"))
    j.at("enable").get_to(outline.enable);

  if (j.contains("idleColor"))
    j.at("idleColor").get_to(outline.idleColor);

  if (j.contains("activeColor"))
    j.at("activeColor").get_to(outline.activeColor);

  if (j.contains("thickness"))
    j.at("thickness").get_to(outline.thickness);
}

void to_json(json &j, const Trail &trail) {
  j = {
      {"enable", trail.enable},     {"duration", trail.duration},
      {"color", trail.color},       {"maxLength", trail.maxLength},
      {"style", trail.style},       {"maxWidth", trail.maxWidth},
      {"minWidth", trail.minWidth}, {"subdivisions", trail.subdivisions},
  };
}

void from_json(const json &j, Trail &trail) {
  if (j.contains("enable"))
    j.at("enable").get_to(trail.enable);

  if (j.contains("duration"))
    j.at("duration").get_to(trail.duration);

  if (j.contains("color"))
    j.at("color").get_to(trail.color);

  if (j.contains("maxLength"))
    j.at("maxLength").get_to(trail.maxLength);

  if (j.contains("style"))
    j.at("style").get_to(trail.style);

  if (j.contains("maxWidth"))
    j.at("maxWidth").get_to(trail.maxWidth);

  if (j.contains("minWidth"))
    j.at("minWidth").get_to(trail.minWidth);

  if (j.contains("subdivisions"))
    j.at("subdivisions").get_to(trail.subdivisions);
}

void to_json(json &j, const MouseTheme &mouseTheme) {
  j = {{"enabled", mouseTheme.enabled},
       {"style", mouseTheme.style},
       {"inner", mouseTheme.inner},
       {"outer", mouseTheme.outer},
       {"fill", mouseTheme.fill},
       {"lerpColor", mouseTheme.lerpColor},
       {"smallRadius", mouseTheme.smallRadius},
       {"outerThickness", mouseTheme.outerThickness},
       {"sensitivity", mouseTheme.sensitivity},
       {"decayRate", mouseTheme.decayRate},
       {"imagesDisplay", mouseTheme.imagesDisplay},
       {"imageOpacity", mouseTheme.imageOpacity},
       {"imageScale", mouseTheme.imageScale},
       {"fadeTime", mouseTheme.fadeTime},
       {"rounding", mouseTheme.rounding},
       {"trail", mouseTheme.trail},
       {"highlightM1", mouseTheme.highlightM1},
       {"highlightColor", mouseTheme.highlightColor},
       {"highlightScale", mouseTheme.higlightScale},
       {"scrollCounter", mouseTheme.scrollCounter}

  };
}

void from_json(const json &j, MouseTheme &mouseTheme) {

  if (j.contains("enabled"))
    j.at("enabled").get_to(mouseTheme.enabled);

  if (j.contains("style"))
    j.at("style").get_to(mouseTheme.style);

  if (j.contains("inner"))
    j.at("inner").get_to(mouseTheme.inner);

  if (j.contains("outer"))
    j.at("outer").get_to(mouseTheme.outer);

  if (j.contains("fill"))
    j.at("fill").get_to(mouseTheme.fill);

  if (j.contains("lerpColor"))
    j.at("lerpColor").get_to(mouseTheme.lerpColor);

  if (j.contains("smallRadius"))
    j.at("smallRadius").get_to(mouseTheme.smallRadius);

  if (j.contains("outerThickness"))
    j.at("outerThickness").get_to(mouseTheme.outerThickness);

  if (j.contains("sensitivity"))
    j.at("sensitivity").get_to(mouseTheme.sensitivity);

  if (j.contains("decayRate"))
    j.at("decayRate").get_to(mouseTheme.decayRate);

  if (j.contains("imagesDisplay"))
    j.at("imagesDisplay").get_to(mouseTheme.imagesDisplay);

  if (j.contains("imageOpacity"))
    j.at("imageOpacity").get_to(mouseTheme.imageOpacity);

  if (j.contains("imageScale"))
    j.at("imageScale").get_to(mouseTheme.imageScale);

  if (j.contains("fadeTime"))
    j.at("fadeTime").get_to(mouseTheme.fadeTime);

  if (j.contains("rounding"))
    j.at("rounding").get_to(mouseTheme.rounding);

  if (j.contains("trail"))
    j.at("trail").get_to(mouseTheme.trail);

  if (j.contains("highlightM1"))
    j.at("highlightM1").get_to(mouseTheme.highlightM1);

  if (j.contains("highlightColor"))
    j.at("highlightColor").get_to(mouseTheme.highlightColor);

  if (j.contains("higlightScale"))
    j.at("higlightScale").get_to(mouseTheme.higlightScale);
}

void to_json(json &j, const AnimType &animType) {
  j = static_cast<int>(animType);
}

void from_json(const json &j, AnimType &animType) {
  animType = static_cast<AnimType>(j.get<int>());
}

void to_json(json &j, const Theme &theme) {
  j = {
      {"idleFill", theme.idleFill},
      {"activeFill", theme.activeFill},
      {"idleText", theme.idleText},
      {"activeText", theme.activeText},
      {"keyRounding", theme.keyRounding},
      {"keyOutline", theme.keyOutline},
      {"obsBackground", theme.obsBackground},
      {"mouse", theme.mouse},
      {"keysImagesDisplay", theme.keysImagesDisplay},
      {"keysImageOpacity", theme.keysImageOpacity},
      {"animation", theme.animation},
      {"pressScale", theme.pressScale},
      {"animationSpeed", theme.animationSpeed},
      {"animateColors", theme.animateColors},
      {"animType", theme.animType},
  };
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

  if (j.contains("mouse"))
    j.at("mouse").get_to(theme.mouse);

  if (j.contains("keysImagesDisplay"))
    j.at("keysImagesDisplay").get_to(theme.keysImagesDisplay);

  if (j.contains("keysImageOpacity"))
    j.at("keysImageOpacity").get_to(theme.keysImageOpacity);

  if (j.contains("animation"))
    j.at("animation").get_to(theme.animation);

  if (j.contains("pressScale"))
    j.at("pressScale").get_to(theme.pressScale);

  if (j.contains("animationSpeed"))
    j.at("animationSpeed").get_to(theme.animationSpeed);

  if (j.contains("animateColors"))
    j.at("animateColors").get_to(theme.animateColors);

  if (j.contains("animType"))
    j.at("animType").get_to(theme.animType);
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
       {"overlay", settings.overlay},
       {"font", settings.font},
       {"misc", settings.misc},
       {"configName", settings.configName},
       {"imagePack", settings.imagePack}};
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

  if (j.contains("overlay"))
    j.at("overlay").get_to(settings.overlay);

  if (j.contains("font"))
    j.at("font").get_to(settings.font);

  if (j.contains("misc"))
    j.at("misc").get_to(settings.misc);

  if (j.contains("configName"))
    j.at("configName").get_to(settings.configName);

  if (j.contains("imagePack"))
    j.at("imagePack").get_to(settings.imagePack);
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

void to_json(json &j, const Overlay &overlay) {
  j = {{"x", overlay.x}, {"y", overlay.y}, {"scale", overlay.scale}};
}

void from_json(const json &j, Overlay &overlay) {
  if (j.contains("x"))
    j.at("x").get_to(overlay.x);
  if (j.contains("y"))
    j.at("y").get_to(overlay.y);
  if (j.contains("scale"))
    j.at("scale").get_to(overlay.scale);
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

void to_json(json &j, const TrailType &type) { j = static_cast<int>(type); }

void from_json(const json &j, TrailType &type) {
  type = static_cast<TrailType>(j.get<int>());
}

void to_json(json &j, const TrailStyle &trailStyle) {
  j = {{"name", trailStyle.name}, {"type", trailStyle.type}};
}

void from_json(const json &j, TrailStyle &trailStyle) {
  if (j.contains("name"))
    j.at("name").get_to(trailStyle.name);

  if (j.contains("type"))
    j.at("type").get_to(trailStyle.type);
}
} // namespace Mouse

namespace Layouts {
void to_json(nlohmann::json &j, const Key &key) {
  j = {{"type", key.type},   {"keyCode", key.code},
       {"label", key.label}, {"shiftLabel", key.shiftLabel},
       {"x", key.x},         {"y", key.y},
       {"w", key.w},         {"h", key.h}};
}

void from_json(const nlohmann::json &j, Key &key) {
  if (j.contains("type"))
    j.at("type").get_to(key.type);

  if (j.contains("keyCode"))
    j.at("keyCode").get_to(key.code);

  if (j.contains("label"))
    j.at("label").get_to(key.label);

  if (j.contains("shiftLabel"))
    j.at("shiftLabel").get_to(key.shiftLabel);

  if (j.contains("x"))
    j.at("x").get_to(key.x);

  if (j.contains("y"))
    j.at("y").get_to(key.y);

  if (j.contains("w"))
    j.at("w").get_to(key.w);

  if (j.contains("h"))
    j.at("h").get_to(key.h);
}

void to_json(nlohmann::json &j, const Mouse &mouse) {
  j = {
      {"centerX", mouse.centerX}, {"centerY", mouse.centerY},
      {"width", mouse.width},     {"height", mouse.height},
      {"radius", mouse.radius},
  };
}

void from_json(const nlohmann::json &j, Mouse &mouse) {
  if (j.contains("centerX"))
    j.at("centerX").get_to(mouse.centerX);

  if (j.contains("centerY"))
    j.at("centerY").get_to(mouse.centerY);

  if (j.contains("width"))
    j.at("width").get_to(mouse.width);

  if (j.contains("height"))
    j.at("height").get_to(mouse.height);

  if (j.contains("radius"))
    j.at("radius").get_to(mouse.radius);
}

void to_json(nlohmann::json &j, const Layout &layout) {
  j = {
      {"windowWidth", layout.windowWidth},
      {"windowHeight", layout.windowHeight},
      {"mouse", layout.mouse},
      {"keys", layout.keys},
  };
}

void from_json(const nlohmann::json &j, Layout &layout) {
  if (j.contains("windowWidth"))
    j.at("windowWidth").get_to(layout.windowWidth);

  if (j.contains("windowHeight"))
    j.at("windowHeight").get_to(layout.windowHeight);

  if (j.contains("mouse"))
    j.at("mouse").get_to(layout.mouse);

  if (j.contains("keys"))
    j.at("keys").get_to(layout.keys);
}
} // namespace Layouts

namespace Images {

void to_json(json &j, const Image &image) {
  j = {{"backGroundFileName", image.backGroundFileName},
       {"activeFileName", image.activeFileName},
       {"vkCode", image.vkCode}};
}

void from_json(const json &j, Image &image) {
  if (j.contains("backGroundFileName"))
    j.at("backGroundFileName").get_to(image.backGroundFileName);

  if (j.contains("activeFileName"))
    j.at("activeFileName").get_to(image.activeFileName);

  if (j.contains("vkCode"))
    j.at("vkCode").get_to(image.vkCode);
}

void to_json(json &j, const MouseButton &mouseButton) {
  j = {{"x", mouseButton.x},
       {"y", mouseButton.y},
       {"w", mouseButton.w},
       {"h", mouseButton.h},
       {"buttonIdle", mouseButton.buttonIdle},
       {"buttonActive", mouseButton.buttonActive}};
}

void from_json(const json &j, MouseButton &mouseButton) {
  if (j.contains("x"))
    j.at("x").get_to(mouseButton.x);

  if (j.contains("y"))
    j.at("y").get_to(mouseButton.y);

  if (j.contains("w"))
    j.at("w").get_to(mouseButton.w);

  if (j.contains("h"))
    j.at("h").get_to(mouseButton.h);

  if (j.contains("buttonIdle"))
    j.at("buttonIdle").get_to(mouseButton.buttonIdle);

  if (j.contains("buttonActive"))
    j.at("buttonActive").get_to(mouseButton.buttonActive);
}

void to_json(json &j, const Mouse &mouse) {
  j = {{"left", mouse.left},
       {"right", mouse.right},
       {"middle", mouse.middle},
       {"scrollUp", mouse.scrollUp},
       {"scrollDown", mouse.scrollDown},
       {"mouse4", mouse.mouse4},
       {"mouse5", mouse.mouse5},
       {"bottom", mouse.bottom}};
}

void from_json(const json &j, Mouse &mouse) {
  if (j.contains("left"))
    j.at("left").get_to(mouse.left);

  if (j.contains("right"))
    j.at("right").get_to(mouse.right);

  if (j.contains("middle"))
    j.at("middle").get_to(mouse.middle);

  if (j.contains("scrollUp"))
    j.at("scrollUp").get_to(mouse.scrollUp);

  if (j.contains("scrollDown"))
    j.at("scrollDown").get_to(mouse.scrollDown);

  if (j.contains("mouse4"))
    j.at("mouse4").get_to(mouse.mouse4);

  if (j.contains("mouse5"))
    j.at("mouse5").get_to(mouse.mouse5);

  if (j.contains("bottom"))
    j.at("bottom").get_to(mouse.bottom);
}

void to_json(json &j, const Pack &pack) {
  j = {{"name", pack.name},
       {"defaultKey", pack.defaultKey},
       {"explicitImages", pack.explicitImages},
       {"mouse", pack.mouse}};
}

void from_json(const json &j, Pack &pack) {
  if (j.contains("name"))
    j.at("name").get_to(pack.name);

  if (j.contains("defaultKey"))
    j.at("defaultKey").get_to(pack.defaultKey);

  if (j.contains("explicitImages"))
    j.at("explicitImages").get_to(pack.explicitImages);

  if (j.contains("mouse"))
    j.at("mouse").get_to(pack.mouse);
}

} // namespace Images

namespace Binds {

void to_json(json &j, const Bind &bind) {
  j = {{"name", bind.name},
       {"vkCode", bind.vkCode},
       {"keyName", bind.keyName},
       {"type", static_cast<int>(bind.type)}};
}

void from_json(const json &j, Bind &bind) {
  if (j.contains("name"))
    j.at("name").get_to(bind.name);

  if (j.contains("vkCode"))
    j.at("vkCode").get_to(bind.vkCode);

  if (j.contains("keyName"))
    j.at("keyName").get_to(bind.keyName);

  if (j.contains("type"))
    bind.type = static_cast<BindType>(j.at("type").get<int>());
}

} // namespace Binds

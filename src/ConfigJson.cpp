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
  j = {{"idleFill", theme.idleFill}, {"activeFill", theme.activeFill},
       {"idleText", theme.idleText}, {"activeText", theme.activeText},
       {"rounding", theme.rounding}, {"outline", theme.outline}};
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

  if (j.contains("rounding"))
    j.at("rounding").get_to(theme.rounding);

  if (j.contains("outline"))
    j.at("outline").get_to(theme.outline);
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
       {"misc", settings.misc}};
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
}

void to_json(json &j, const Misc &misc) {
  j = {{"vsync", misc.vsync}, {"obsMode", misc.obsMode}};
}

void from_json(const json &j, Misc &misc) {

  if (j.contains("vsync"))
    j.at("vsync").get_to(misc.vsync);

  if (j.contains("obsMode"))
    j.at("obsMode").get_to(misc.obsMode);
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

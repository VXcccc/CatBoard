#ifndef CONFIG_JSON_H
#define CONFIG_JSON_H

#include "Config/Config.h"
#include <nlohmann/json.hpp>

namespace Config {

using json = nlohmann::json;

void to_json(json &j, const Outline &outline);
void from_json(const json &j, Outline &outline);

void to_json(json &j, const Trail &trail);
void from_json(const json &j, Trail &trail);

void to_json(json &j, const MouseTheme &mouseTheme);
void from_json(const json &j, MouseTheme &mouseTheme);

void to_json(json &j, const AnimType &animType);
void from_json(const json &j, AnimType &animType);

void to_json(json &j, const Theme &theme);
void from_json(const json &j, Theme &theme);

void to_json(json &j, const SoundPackSelection &soundPack);
void from_json(const json &j, SoundPackSelection &soundPack);

void to_json(json &j, const OverlayPosition &overlayPos);
void from_json(const json &j, OverlayPosition &overlayPos);

void to_json(json &j, const Misc &misc);
void from_json(const json &j, Misc &misc);

void to_json(json &j, const Settings &settings);
void from_json(const json &j, Settings &settings);

} // namespace Config

namespace Fonts {

using json = nlohmann::json;

void to_json(json &j, const LetterCaseOptions &options);
void from_json(const json &j, LetterCaseOptions &options);

void to_json(json &j, const LetterCase &letterCase);
void from_json(const json &j, LetterCase &letterCase);

void to_json(json &j, const Font &font);
void from_json(const json &j, Font &font);

} // namespace Fonts

namespace Mouse {
using json = nlohmann::json;

void to_json(json &j, const Type &type);
void from_json(const json &j, Type &type);

void to_json(json &j, const MouseStyle &mouseStyle);
void from_json(const json &j, MouseStyle &mouseStyle);

void to_json(json &j, const TrailType &type);
void from_json(const json &j, TrailType &type);

void to_json(json &j, const TrailStyle &trailStyle);
void from_json(const json &j, TrailStyle &trailStyle);

} // namespace Mouse

namespace Layouts {

void to_json(nlohmann::json &j, const Key &key);
void from_json(const nlohmann::json &j, Key &key);

void to_json(nlohmann::json &j, const Mouse &mouse);
void from_json(const nlohmann::json &j, Mouse &mouse);

void to_json(nlohmann::json &j, const Layout &layout);
void from_json(const nlohmann::json &j, Layout &layout);

} // namespace Layouts

namespace Images {
using json = nlohmann::json;

void to_json(json &j, const Image &image);
void from_json(const json &j, Image &image);

void to_json(json &j, const MouseButton &mouseButton);
void from_json(const json &j, MouseButton &mouseButton);

void to_json(json &j, const Mouse &mouse);
void from_json(const json &j, Mouse &mouse);

void to_json(json &j, const Pack &pack);
void from_json(const json &j, Pack &pack);

} // namespace Images
#endif // CONFIG_JSON_H

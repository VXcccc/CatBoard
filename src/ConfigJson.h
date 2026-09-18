#ifndef CONFIG_JSON_H
#define CONFIG_JSON_H

#include "Config.h"
#include <nlohmann/json.hpp>

namespace Config {

using json = nlohmann::json;

void to_json(json &j, const Outline &outline);
void from_json(const json &j, Outline &outline);

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

#endif // CONFIG_JSON_H

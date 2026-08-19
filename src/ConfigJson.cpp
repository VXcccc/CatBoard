#include "ConfigJson.h"
#include <nlohmann/json.hpp>

namespace Config {

void to_json(json& j, const Outline& outline) {
    j = {
        {"enable", outline.enable},
        {"color", outline.color},
        {"thickness", outline.thickness}
    };
}

void from_json(const json& j, Outline& outline) {
    if (j.contains("enable"))
        j.at("enable").get_to(outline.enable);

    if (j.contains("color"))
        j.at("color").get_to(outline.color);

    if (j.contains("thickness"))
        j.at("thickness").get_to(outline.thickness);
}

void to_json(json& j, const Theme& theme) {
    j = {
        {"idleFill", theme.idleFill},
        {"activeFill", theme.activeFill},
        {"idleText", theme.idleText},
        {"activeText", theme.activeText},
        {"rounding", theme.rounding},
        {"outline", theme.outline}
    };
}

void from_json(const json& j, Theme& theme) {
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

void to_json(json& j, const Settings& settings) {
    j = {
        {"version", settings.version},
        {"theme", settings.theme},
        {"selectedLayout", settings.selectedLayout}
    };
}

void from_json(const json& j, Settings& settings) {
    if (j.contains("version"))
        j.at("version").get_to(settings.version);

    if (j.contains("theme"))
        j.at("theme").get_to(settings.theme);

    if (j.contains("selectedLayout"))
        j.at("selectedLayout").get_to(settings.selectedLayout);
}

} // namespace Config

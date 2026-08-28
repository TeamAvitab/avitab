/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2026 Folke Will and Avitab Contributors
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <lvgl.h>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <unordered_map>
#include "Theme.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace avitab {

// ── statische Member ──────────────────────────────────────────
Theme                       Theme::instance;
lv_theme_t                  Theme::lvTheme    = {};
lv_display_t *              Theme::display    = nullptr;
std::string                 Theme::themesDir;
std::string                 Theme::activeName;
bool                        Theme::initialized = false;
std::vector<Theme::ChangeCallback> Theme::listeners;

// ── init ──────────────────────────────────────────────────────
void Theme::init(const std::string & dir, lv_display_t * disp) {
    themesDir = dir;
    display   = disp;

    // LVGL-Theme einmalig registrieren
    lv_theme_set_apply_cb(&lvTheme, themeApplyCb);
    lv_theme_set_parent  (&lvTheme, lv_display_get_theme(disp));
    lv_display_set_theme (disp, &lvTheme);

    load("default");
}

// ── load (nach Name) ──────────────────────────────────────────
void Theme::load(const std::string & name) {
    std::string path = themesDir + "/" + name + ".json";
    loadFromPath(path);
    activeName = name;
}

// ── loadFromPath ──────────────────────────────────────────────
void Theme::loadFromPath(const std::string & jsonPath) {
    parseJson(jsonPath);
    initialized = true;

    // Alle existierenden Widgets neu stylen
    lv_obj_report_style_change(nullptr);

    notifyListeners();
}

// ── available ─────────────────────────────────────────────────
std::vector<std::string> Theme::available() {
    std::vector<std::string> names;
    if (!fs::exists(themesDir)) return names;

    for (auto & entry : fs::directory_iterator(themesDir)) {
        if (entry.path().extension() == ".json") {
            names.push_back(entry.path().stem().string());
        }
    }
    return names;
}

const std::string & Theme::currentName() {
    return activeName;
}

// ── Singleton ─────────────────────────────────────────────────
Theme & Theme::active() {
    if (!initialized) {
        throw std::runtime_error("Theme::init() muss zuerst aufgerufen werden");
    }
    return instance;
}

// ── onChange ──────────────────────────────────────────────────
void Theme::onChange(ChangeCallback cb) {
    listeners.push_back(std::move(cb));
}

void Theme::notifyListeners() {
    for (auto & cb : listeners) {
        cb(instance);
    }
}

// ── JSON parsen ───────────────────────────────────────────────
void Theme::parseJson(const std::string & path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Theme: Datei nicht gefunden: " + path);
    }

    json j = json::parse(f, nullptr, /*exceptions=*/true, /*ignore_comments=*/true);

    auto & c  = instance.colors;
    auto & jc = j.at("colors");
    c.background     = parseColor(jc.at("background"));
    c.surface        = parseColor(jc.at("surface"));
    c.primary        = parseColor(jc.at("primary"));
    c.primaryPressed = parseColor(jc.at("primaryPressed"));
    c.onPrimary      = parseColor(jc.at("onPrimary"));
    c.text           = parseColor(jc.at("text"));
    c.textMuted      = parseColor(jc.at("textMuted"));
    c.border         = parseColor(jc.at("border"));
    c.error          = parseColor(jc.at("error"));

    auto & fo = instance.fonts;
    auto & jf = j.at("fonts");
    fo.small   = parseFont(jf.at("small"));
    fo.body    = parseFont(jf.at("body"));
    fo.heading = parseFont(jf.at("heading"));

    auto & s  = instance.spacing;
    auto & js = j.at("spacing");
    s.paddingSmall  = js.at("paddingSmall");
    s.paddingNormal = js.at("paddingNormal");
    s.paddingLarge  = js.at("paddingLarge");
    s.borderRadius  = js.at("borderRadius");
    s.borderWidth   = js.at("borderWidth");
}

// ── LVGL-Callback ─────────────────────────────────────────────
void Theme::themeApplyCb(lv_theme_t * /*th*/, lv_obj_t * obj) {
    if (!initialized) return;
    Theme & t = instance;
    const lv_obj_class_t * cls = lv_obj_get_class(obj);

    if (cls == &lv_obj_class) {
        lv_obj_set_style_bg_color    (obj, t.colors.surface,      0);
        lv_obj_set_style_text_color  (obj, t.colors.text,         0);
        lv_obj_set_style_text_font   (obj, t.fonts.body,          0);
        lv_obj_set_style_border_color(obj, t.colors.border,       0);
        lv_obj_set_style_border_width(obj, t.spacing.borderWidth,  0);
        lv_obj_set_style_radius      (obj, t.spacing.borderRadius, 0);
        lv_obj_set_style_pad_all     (obj, t.spacing.paddingNormal,0);
    }
    else if (cls == &lv_button_class) {
        lv_obj_set_style_bg_color  (obj, t.colors.primary,        0);
        lv_obj_set_style_bg_color  (obj, t.colors.primaryPressed,  LV_STATE_PRESSED);
        lv_obj_set_style_text_color(obj, t.colors.onPrimary,      0);
        lv_obj_set_style_radius    (obj, t.spacing.borderRadius,   0);
        lv_obj_set_style_pad_hor   (obj, t.spacing.paddingNormal,  0);
        lv_obj_set_style_pad_ver   (obj, t.spacing.paddingSmall,   0);
    }
    else if (cls == &lv_label_class) {
        lv_obj_set_style_text_font (obj, t.fonts.body,  0);
        lv_obj_set_style_text_color(obj, t.colors.text, 0);
    }
    else if (cls == &lv_textarea_class) {
        lv_obj_set_style_bg_color    (obj, t.colors.surface,  0);
        lv_obj_set_style_text_color  (obj, t.colors.text,     0);
        lv_obj_set_style_text_font   (obj, t.fonts.body,      0);
        lv_obj_set_style_border_color(obj, t.colors.primary,  LV_STATE_FOCUSED);
    }
}

// ── parseColor ────────────────────────────────────────────────
lv_color_t Theme::parseColor(const std::string & hex) {
    std::string s = hex;
    if (!s.empty() && s[0] == '#') s.erase(0, 1);
    if (s.size() != 6) {
        throw std::runtime_error("Theme: ungültiger Farbwert: " + hex);
    }
    return lv_color_hex(std::stoul(s, nullptr, 16));
}

// ── parseFont ─────────────────────────────────────────────────
const lv_font_t * Theme::parseFont(const std::string & name) {
    static const std::unordered_map<std::string, const lv_font_t *> map = {
//        { "montserrat_8",  &lv_font_montserrat_8  },
        { "montserrat_10", &lv_font_montserrat_10 },
        { "montserrat_12", &lv_font_montserrat_12 },
        { "montserrat_14", &lv_font_montserrat_14 },
        { "montserrat_16", &lv_font_montserrat_16 },
//        { "montserrat_18", &lv_font_montserrat_18 },
//        { "montserrat_20", &lv_font_montserrat_20 },
//        { "montserrat_22", &lv_font_montserrat_22 },
//        { "montserrat_24", &lv_font_montserrat_24 },
//        { "montserrat_28", &lv_font_montserrat_28 },
//        { "montserrat_32", &lv_font_montserrat_32 },
//        { "montserrat_36", &lv_font_montserrat_36 },
//        { "montserrat_48", &lv_font_montserrat_48 },
//        { "unscii_8",      &lv_font_unscii_8      },
//        { "unscii_16",     &lv_font_unscii_16     },
    };
    auto it = map.find(name);
    if (it == map.end()) {
        throw std::runtime_error("Theme: unbekannter Font: " + name);
    }
    return it->second;
}

} // namespace avitab
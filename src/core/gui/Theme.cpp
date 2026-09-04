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
#include <utility>
#include "Theme.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace avitab {

Theme::Theme(std::string dir, lv_display_t *disp):
    themesDir(std::move(dir)),
    display(disp)
{
    initStyles();

    // Register our theme on the display, chained after the current one.
    lvTheme.user_data = this;
    lv_theme_set_apply_cb(&lvTheme, &Theme::applyThemeTrampoline);
    lv_theme_set_parent(&lvTheme, lv_display_get_theme(disp));
    lv_display_set_theme(disp, &lvTheme);

    load("default");
}

// ── load (by name) ───────────────────────────────────────────
void Theme::load(const std::string &name) {
    std::string path = themesDir + "/" + name + ".json";
    loadFromPath(path);
    activeName = name;
}

// ── loadFromPath ─────────────────────────────────────────────
void Theme::loadFromPath(const std::string &jsonPath) {
    parseJson(jsonPath);

    // Repopulate the shared style objects from the freshly parsed values, then
    // tell LVGL they changed so every widget that references them is refreshed.
    rebuildStyles();
    lv_obj_report_style_change(nullptr);

    notifyListeners();
}

// ── available ────────────────────────────────────────────────
std::vector<std::string> Theme::available() {
    std::vector<std::string> names;
    if (!fs::exists(themesDir)) return names;

    for (auto &entry : fs::directory_iterator(themesDir)) {
        if (entry.path().extension() == ".json") {
            names.push_back(entry.path().stem().string());
        }
    }
    return names;
}

const std::string &Theme::currentName() const {
    return activeName;
}

// ── onChange ─────────────────────────────────────────────────
void Theme::onChange(ChangeCallback cb) {
    listeners.push_back(std::move(cb));
}

void Theme::notifyListeners() {
    for (auto &cb : listeners) {
        cb(*this);
    }
}

// ── JSON parsing ─────────────────────────────────────────────
void Theme::parseJson(const std::string &path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Theme: file not found: " + path);
    }

    json j = json::parse(f, nullptr, /*exceptions=*/true, /*ignore_comments=*/true);

    auto &c  = colors;
    auto &jc = j.at("colors");
    c.background     = parseColor(jc.at("background"));
    c.surface        = parseColor(jc.at("surface"));
    c.primary        = parseColor(jc.at("primary"));
    c.primaryPressed = parseColor(jc.at("primaryPressed"));
    c.onPrimary      = parseColor(jc.at("onPrimary"));
    c.hovered        = parseColor(jc.at("hovered"));
    c.text           = parseColor(jc.at("text"));
    c.textMuted      = parseColor(jc.at("textMuted"));
    c.border         = parseColor(jc.at("border"));
    c.error          = parseColor(jc.at("error"));
    c.appButtonBackground = parseColor(jc.at("appButtonBackground"));

    auto &fo = fonts;
    auto &jf = j.at("fonts");
    fo.small   = parseFont(jf.at("small"));
    fo.body    = parseFont(jf.at("body"));
    fo.heading = parseFont(jf.at("heading"));

    auto &s  = spacing;
    auto &js = j.at("spacing");
    s.paddingSmall  = js.at("paddingSmall");
    s.paddingNormal = js.at("paddingNormal");
    s.paddingLarge  = js.at("paddingLarge");
    s.borderRadius  = js.at("borderRadius");
    s.borderWidth   = js.at("borderWidth");
}

// ── LVGL callback ────────────────────────────────────────────
void Theme::applyThemeTrampoline(lv_theme_t *th, lv_obj_t *obj) {
    auto *self = static_cast<Theme *>(th->user_data);
    if (self) {
        self->applyTheme(obj);
    }
}

void Theme::applyTheme(lv_obj_t *obj) {
    if (!stylesInited) return;
    const lv_obj_class_t *cls = lv_obj_get_class(obj);

    if (cls == &lv_obj_class) {
        lv_obj_add_style(obj, &styles.obj, 0);
    }
    else if (cls == &lv_button_class) {
        lv_obj_add_style(obj, &styles.button,        0);
        lv_obj_add_style(obj, &styles.buttonPressed, LV_STATE_PRESSED);
        lv_obj_add_style(obj, &styles.buttonHovered, LV_STATE_HOVERED);
    }
    else if (cls == &lv_keyboard_class) {
        lv_obj_add_style(obj, &styles.button,        0);
    }
    else if (cls == &lv_label_class) {
        lv_obj_add_style(obj, &styles.label, 0);
    }
    else if (cls == &lv_textarea_class) {
        lv_obj_add_style(obj, &styles.textarea,        0);
        lv_obj_add_style(obj, &styles.textareaFocused, LV_STATE_FOCUSED);
    }
    else if (cls == &lv_dropdown_class) {
        lv_obj_add_style(obj, &styles.obj, 0);
    }
    else if (cls == &lv_dropdownlist_class) {
        lv_obj_add_style(obj, &styles.obj, 0);
    }
}

// ── shared style objects ─────────────────────────────────────
void Theme::initStyles() {
    lv_style_init(&styles.obj);
    lv_style_init(&styles.button);
    lv_style_init(&styles.buttonPressed);
    lv_style_init(&styles.buttonHovered);
    lv_style_init(&styles.label);
    lv_style_init(&styles.textarea);
    lv_style_init(&styles.textareaFocused);
    stylesInited = true;
}

void Theme::rebuildStyles() {
    // Base object: a flat surface. Individual widget classes opt into borders,
    // radius and padding below.
    lv_style_reset(&styles.obj);
    lv_style_set_bg_color    (&styles.obj, colors.surface);
    lv_style_set_text_color  (&styles.obj, colors.text);
    lv_style_set_text_font   (&styles.obj, fonts.body);
    lv_style_set_border_color(&styles.obj, colors.border);
    lv_style_set_border_width(&styles.obj, 0);
    lv_style_set_radius      (&styles.obj, 0);
    lv_style_set_pad_all     (&styles.obj, spacing.paddingNone);

    lv_style_reset(&styles.button);
    lv_style_set_bg_color  (&styles.button, colors.primary);
    lv_style_set_text_color(&styles.button, colors.onPrimary);
    lv_style_set_radius    (&styles.button, spacing.borderRadius);
    lv_style_set_pad_hor   (&styles.button, spacing.paddingNormal);
    lv_style_set_pad_ver   (&styles.button, spacing.paddingSmall);

    lv_style_reset(&styles.buttonPressed);
    lv_style_set_bg_color(&styles.buttonPressed, colors.primaryPressed);
    lv_style_reset(&styles.buttonHovered);
    lv_style_set_bg_color(&styles.buttonHovered, colors.hovered);

    lv_style_reset(&styles.label);
    lv_style_set_text_font (&styles.label, fonts.body);
    lv_style_set_text_color(&styles.label, colors.text);

    lv_style_reset(&styles.textarea);
    lv_style_set_bg_color  (&styles.textarea, colors.surface);
    lv_style_set_text_color(&styles.textarea, colors.text);
    lv_style_set_text_font (&styles.textarea, fonts.body);

    lv_style_reset(&styles.textareaFocused);
    lv_style_set_border_color(&styles.textareaFocused, colors.border);
}

void Theme::setLocalStyle(lv_obj_t* lvObj, const std::string style) {
    if (style == "UIContainer" ) {
        lv_obj_set_style_pad_hor(lvObj, spacing.paddingNormal, 0);
        lv_obj_set_style_pad_ver(lvObj, spacing.paddingNormal, 0);
    }
    else if (style == "WindowContent") {
        lv_obj_set_style_pad_hor(lvObj, spacing.paddingSmall, 0);
        lv_obj_set_style_pad_ver(lvObj, spacing.paddingSmall, 0);
    }
    else if (style == "AppButton" ) {
        lv_obj_set_style_bg_color(lvObj, colors.appButtonBackground, 0);
    }
}

// ── parseColor ───────────────────────────────────────────────
lv_color_t Theme::parseColor(const std::string &hex) {
    std::string s = hex;
    if (!s.empty() && s[0] == '#') s.erase(0, 1);
    if (s.size() != 6) {
        throw std::runtime_error("Theme: invalid color value: " + hex);
    }
    return lv_color_hex(std::stoul(s, nullptr, 16));
}

// ── parseFont ────────────────────────────────────────────────
const lv_font_t *Theme::parseFont(const std::string &name) {
    static const std::unordered_map<std::string, const lv_font_t *> map = {
        { "montserrat_10", &lv_font_montserrat_10 },
        { "montserrat_12", &lv_font_montserrat_12 },
        { "montserrat_14", &lv_font_montserrat_14 },
        { "montserrat_16", &lv_font_montserrat_16 },
        { "montserrat_18", &lv_font_montserrat_18 },
        { "montserrat_20", &lv_font_montserrat_20 },
    };
    auto it = map.find(name);
    if (it == map.end()) {
        throw std::runtime_error("Theme: unknown font: " + name);
    }
    return it->second;
}

} // namespace avitab
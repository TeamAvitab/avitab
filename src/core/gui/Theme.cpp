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
#include "Logger.h"
#include "Theme.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace avitab {

Theme::Theme(std::string dir, std::string name, lv_display_t *disp):
    themesDir(std::move(dir)),
    display(disp)
{
    initStyles();

    // Register our theme on the display, chained after the current one.
    lvTheme.user_data = this;
    lv_theme_set_apply_cb(&lvTheme, &Theme::applyThemeTrampoline);
    lv_theme_set_parent(&lvTheme, lv_display_get_theme(disp));
    lv_display_set_theme(disp, &lvTheme);

    load(name);
}

// ── load (by name) ───────────────────────────────────────────
void Theme::load(const std::string &name) {
    logger::verbose("GUI: loading Theme %s", name.c_str());
    std::string path = themesDir + "/" + name + "/" + name + ".json";
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
        throw std::runtime_error("GUI: Theme file not found: " + path);
    }

    json j = json::parse(f, nullptr, /*exceptions=*/true, /*ignore_comments=*/true);

    auto &i  = icons;
    auto &ji = j.at("icons");
    i.Charts      = ji.at("charts");
    i.Airports    = ji.at("airports");
    i.Routes      = ji.at("routes");
    i.Maps        = ji.at("maps");
    i.PlaneManual = ji.at("planemanual");
    i.Notes       = ji.at("notes");
    i.Providers   = ji.at("providers");
    i.About       = ji.at("about");

    auto &c  = colors;
    auto &jc = j.at("colors");
    c.background     = parseColor(jc.at("background"));
    c.surface        = parseColor(jc.at("surface"));
    c.headerArea     = parseColor(jc.at("headerArea"));
    c.primary        = parseColor(jc.at("primary"));
    c.primaryPressed = parseColor(jc.at("primaryPressed"));
    c.onPrimary      = parseColor(jc.at("onPrimary"));
    c.hovered        = parseColor(jc.at("hovered"));
    c.headerText     = parseColor(jc.at("headerText"));
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
        lv_obj_add_style(obj, &styles.obj, LV_PART_MAIN);
    }
    else if (cls == &lv_checkbox_class) {
        lv_obj_add_style(obj, &styles.button, LV_PART_MAIN);
        lv_obj_add_style(obj, &styles.button, LV_PART_INDICATOR);
    }
    else if (cls == &lv_button_class) {
        lv_obj_add_style(obj, &styles.button, LV_PART_MAIN);
    }
    else if (cls == &lv_keyboard_class) {
        lv_obj_add_style(obj, &styles.button, LV_PART_MAIN);
        lv_obj_add_style(obj, &styles.keyboardItems, LV_PART_ITEMS);
    }
    else if (cls == &lv_label_class) {
        lv_obj_add_style(obj, &styles.label, LV_PART_MAIN);
    }
    else if (cls == &lv_textarea_class) {
        lv_obj_add_style(obj, &styles.textarea, LV_PART_MAIN);
        lv_obj_add_style(obj, &styles.textareaFocused, LV_STATE_FOCUSED);
    }
    else if (cls == &lv_dropdown_class) {
        lv_obj_add_style(obj, &styles.dropdown, LV_PART_MAIN);
        lv_obj_add_style(obj, &styles.textareaFocused, LV_STATE_FOCUSED);
    }
    else if (cls == &lv_dropdownlist_class) {
        lv_obj_add_style(obj, &styles.dropdownlist, LV_PART_MAIN);
        lv_obj_add_style(obj, &styles.textareaFocused, LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &styles.textareaFocused, LV_PART_SELECTED);
    }
    else if (cls == &lv_slider_class) {
        lv_obj_add_style(obj, &styles.button, LV_PART_MAIN | LV_PART_INDICATOR | LV_PART_KNOB);
    }
}

// ── shared style objects ─────────────────────────────────────
void Theme::initStyles() {
    lv_style_init(&styles.obj);
    lv_style_init(&styles.button);
    lv_style_init(&styles.buttonPressed);
    lv_style_init(&styles.buttonHovered);
    lv_style_init(&styles.keyboardItems);
    lv_style_init(&styles.label);
    lv_style_init(&styles.textarea);
    lv_style_init(&styles.textareaFocused);
    lv_style_init(&styles.dropdown);
    lv_style_init(&styles.dropdownlist);
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
    lv_style_set_text_color(&styles.button, colors.headerText);
    lv_style_set_radius    (&styles.button, spacing.borderRadius);
    lv_style_set_pad_hor   (&styles.button, spacing.paddingNormal);
    lv_style_set_pad_ver   (&styles.button, spacing.paddingSmall);

    lv_style_reset(&styles.buttonPressed);
    lv_style_set_bg_color(&styles.buttonPressed, colors.primaryPressed);
    lv_style_reset(&styles.buttonHovered);
    lv_style_set_bg_color(&styles.buttonHovered, colors.hovered);

    lv_style_reset(&styles.keyboardItems);
    lv_style_set_text_color(&styles.keyboardItems, colors.text);

    lv_style_reset(&styles.label);
    lv_style_set_text_font (&styles.label, fonts.body);
    lv_style_set_text_color(&styles.label, colors.text);

    lv_style_reset(&styles.textarea);
    lv_style_set_bg_color  (&styles.textarea, colors.surface);
    lv_style_set_text_color(&styles.textarea, colors.text);
    lv_style_set_text_font (&styles.textarea, fonts.body);

    lv_style_reset(&styles.textareaFocused);
    lv_style_set_border_color(&styles.textareaFocused, colors.border);
    lv_style_set_bg_color(&styles.textareaFocused, colors.hovered);

    lv_style_reset(&styles.dropdown);
    lv_style_set_bg_color  (&styles.dropdown, colors.surface);
    lv_style_set_text_color(&styles.dropdown, colors.text);
    lv_style_set_radius    (&styles.dropdown, spacing.borderRadius);
    lv_style_set_pad_hor   (&styles.dropdown, spacing.paddingNormal);
    lv_style_set_pad_ver   (&styles.dropdown, spacing.paddingSmall);

    lv_style_reset(&styles.dropdownlist);
    lv_style_set_bg_color    (&styles.dropdownlist, colors.appButtonBackground);
    lv_style_set_border_color(&styles.dropdownlist, colors.border);
    lv_style_set_radius      (&styles.dropdownlist, spacing.borderRadius);
    lv_style_set_pad_hor     (&styles.dropdownlist, spacing.paddingNormal);
    lv_style_set_pad_ver     (&styles.dropdownlist, spacing.paddingSmall);

    lv_style_reset(&styles.headerArea);
    lv_style_set_bg_color  (&styles.headerArea, colors.headerArea);
    lv_style_set_text_color(&styles.headerArea, colors.headerText);

    lv_style_reset(&styles.windowContent);
    lv_style_set_pad_hor   (&styles.windowContent, spacing.paddingSmall);
    lv_style_set_pad_ver   (&styles.windowContent, spacing.paddingSmall);
    lv_style_set_text_color(&styles.windowContent, colors.textMuted);
}

void Theme::setLocalStyle(lv_obj_t* obj, const std::string style) {
    if (style == "uiContainer" ) {
        lv_obj_set_style_pad_hor(obj, spacing.paddingNormal, LV_PART_MAIN);
        lv_obj_set_style_pad_ver(obj, spacing.paddingNormal, LV_PART_MAIN);
    }
    else if (style == "windowContent") {
        lv_obj_add_style(obj, &styles.windowContent, LV_PART_MAIN);
    }
    else if (style == "appButton" ) {
        lv_obj_set_style_bg_color(obj, colors.appButtonBackground, LV_PART_MAIN);
        lv_obj_set_style_text_color(obj, colors.text, LV_PART_MAIN);
        lv_obj_add_style(obj, &styles.buttonHovered, LV_STATE_HOVERED);
    }
    else if (style == "headerArea") {
        lv_obj_add_style(obj, &styles.headerArea, LV_PART_MAIN);
    }
    else if (style == "settings") {
        lv_obj_add_style(obj, &styles.dropdownlist, LV_PART_MAIN);
        lv_obj_set_style_border_width(obj, spacing.borderWidth, LV_PART_MAIN);
        lv_obj_set_style_radius(obj, spacing.borderRadius, LV_PART_MAIN);
    }
    else {
        throw std::runtime_error("GUI: Theme::setLocalStyle called with unknown style:" + style);
    }
}

// ── parseColor ───────────────────────────────────────────────
lv_color_t Theme::parseColor(const std::string &hex) {
    std::string s = hex;
    if (!s.empty() && s[0] == '#') s.erase(0, 1);
    if (s.size() != 6) {
        throw std::runtime_error("GUI: invalid Theme color value: " + hex);
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
        throw std::runtime_error("GUI: unknown Theme font: " + name);
    }
    return it->second;
}

} // namespace avitab
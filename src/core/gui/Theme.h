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
#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <memory>
#include <lvgl.h>

namespace avitab {

// enable_shared_from_this lets a holder of the raw Theme* stored in the LVGL
// theme's user_data (see Screen) recover a shared_ptr that shares ownership
// with the one held by LVGLToolkit.
class Theme : public std::enable_shared_from_this<Theme> {
public:
    Theme(std::string themesDir, std::string themeName, lv_display_t *display);

    // Non-copyable / non-movable: the instance registers the address of its
    // lvTheme member with LVGL and stores `this` in lvTheme.user_data.
    // There is exactly one Theme, owned by LVGLToolkit.
    Theme(const Theme &) = delete;
    Theme &operator=(const Theme &) = delete;

    struct Icons {
        inline static std::string Charts;
        inline static std::string Airports;
        inline static std::string Routes;
        inline static std::string Maps;
        inline static std::string PlaneManual;
        inline static std::string Notes;
        inline static std::string Providers;
        inline static std::string About;
    };

    struct Colors {
        lv_color_t background;
        lv_color_t surface;
        lv_color_t headerArea;
        lv_color_t primary;
        lv_color_t primaryPressed;
        lv_color_t onPrimary;
        lv_color_t hovered;
        lv_color_t focused;
        lv_color_t headerText;
        lv_color_t text;
        lv_color_t textMuted;
        lv_color_t border;
        lv_color_t error;
        lv_color_t appButtonBackground;
    };

    struct Fonts {
        const lv_font_t *small;
        const lv_font_t *body;
        const lv_font_t *header;
        const lv_font_t *heading;
    };

    struct Spacing {
        int32_t paddingNone;
        int32_t paddingSmall;
        int32_t paddingNormal;
        int32_t paddingLarge;
        int32_t borderRadius;
        int32_t borderWidth;
    };

    Icons   icons;
    Colors  colors;
    Fonts   fonts;
    Spacing spacing;

    // ── Lifecycle ─────────────────────────────────────────────

    // Switch theme at runtime — triggers all onChange callbacks.
    // name = file name without .json, e.g. "dark"
    void load(const std::string &name);

    // Direct path (for absolute paths / tests)
    void loadFromPath(const std::string &jsonPath);

    // All available theme names (*.json in the themes directory)
    std::vector<std::string> available();

    // Name of the currently loaded theme
    const std::string &currentName() const;

    void setLocalStyle(lv_obj_t* obj, const std::string style);

    // ── Change listeners ──────────────────────────────────────
    // Widgets/apps can register to react to theme changes
    // (e.g. to re-set their own canvas colors).
    using ChangeCallback = std::function<void(const Theme &)>;
    void onChange(ChangeCallback cb);

private:
    void parseJson(const std::string &jsonPath);
    void notifyListeners();

    static lv_color_t parseColor(const std::string &hex);

    // Attaches the shared style objects to a newly created widget, by class.
    // Called once per object at creation time (via the LVGL theme mechanism).
    void applyTheme(lv_obj_t *obj);
    // C trampoline LVGL can call; recovers `this` from lvTheme.user_data.
    static void applyThemeTrampoline(lv_theme_t *th, lv_obj_t *obj);

    // The theme is expressed as a set of shared lv_style_t objects that widgets
    // reference. initStyles() runs once; rebuildStyles() repopulates them from
    // the current colors/fonts/spacing on every (re)load, so a runtime theme
    // switch updates every existing widget. A widget's own local style always
    // takes precedence over these.
    struct Styles {
        lv_style_t obj {};
        lv_style_t button {};
        lv_style_t buttonPressed {};
        lv_style_t buttonHovered {};
        lv_style_t keyboardItems{};
        lv_style_t label {};
        lv_style_t textarea {};
        lv_style_t textareaFocused {};
        lv_style_t dropdown {};
        lv_style_t dropdownlist {};
        lv_style_t headerArea {};
        lv_style_t windowContent {};
        lv_style_t backdrop {};
    };
    void initStyles();
    void rebuildStyles();

    Styles        styles;
    bool          stylesInited = false;
    lv_theme_t    lvTheme {};
    lv_display_t *display = nullptr;
    std::filesystem::path themesDir;
    std::string   activeName;
    std::string   backdrop;
    std::filesystem::path backdropPath;
    std::vector<ChangeCallback> listeners;
};

} // namespace avitab

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
#include <functional>

namespace avitab {

class Theme {
public:
    struct Colors {
        lv_color_t background;
        lv_color_t surface;
        lv_color_t primary;
        lv_color_t primaryPressed;
        lv_color_t onPrimary;
        lv_color_t text;
        lv_color_t textMuted;
        lv_color_t border;
        lv_color_t error;
    };

    struct Fonts {
        const lv_font_t * small;
        const lv_font_t * body;
        const lv_font_t * heading;
    };

    struct Spacing {
        int32_t paddingSmall;
        int32_t paddingNormal;
        int32_t paddingLarge;
        int32_t borderRadius;
        int32_t borderWidth;
    };

    Colors  colors;
    Fonts   fonts;
    Spacing spacing;

    // ── Lebenszyklus ──────────────────────────────────────────

    // Einmalig beim Start — setzt den Theme-Verzeichnis-Pfad
    // und lädt das Default-Theme
    static void init(const std::string & themesDir,
                     lv_display_t      * display);

    // Wechselt zur Laufzeit — löst alle onChange-Callbacks aus
    // name = Dateiname ohne .json, z.B. "dark"
    static void load(const std::string & name);

    // Direkter Pfad (für absolute Pfade / Tests)
    static void loadFromPath(const std::string & jsonPath);

    // Gibt alle verfügbaren Theme-Namen zurück (*.json im themes-Verzeichnis)
    static std::vector<std::string> available();

    // Name des aktuell geladenen Themes
    static const std::string & currentName();

    // ── Singleton-Zugriff ─────────────────────────────────────
    static Theme & active();

    // ── Change-Listener ──────────────────────────────────────
    // Widgets/Apps können sich registrieren um auf Theme-Wechsel
    // zu reagieren (z.B. um eigene Canvas-Farben neu zu setzen)
    using ChangeCallback = std::function<void(const Theme &)>;
    static void onChange(ChangeCallback cb);

private:
    Theme() = default;

    static void parseJson(const std::string & jsonPath);
    static void notifyListeners();

    static lv_color_t        parseColor(const std::string & hex);
    static const lv_font_t * parseFont(const std::string & name);

    static Theme                    instance;
    static lv_theme_t               lvTheme;
    static lv_display_t *           display;
    static std::string              themesDir;
    static std::string              activeName;
    static bool                     initialized;
    static std::vector<ChangeCallback> listeners;

    static void themeApplyCb(lv_theme_t * th, lv_obj_t * obj);
};

} // namespace avitab
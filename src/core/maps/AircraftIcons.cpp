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

#include "AircraftIcons.h"
#include <fmt/core.h>
#include <lunasvg.h>
#include <cassert>

namespace maps {

AircraftIcons *AircraftIcons::factory(uint32_t colorBelow, uint32_t colorSame, uint32_t colorAbove) {
    if (!singleton) {
        singleton = new AircraftIcons(colorBelow, colorSame, colorAbove);
    }
    return singleton;
}

AircraftIcons::AircraftIcons(uint32_t b, uint32_t s, uint32_t a)
:   colorBelow(b), colorSame(s), colorAbove(a)
{
    createIcon(&userPlaneIcon, 28, img::COLOR_LIGHT_RED);
    createIcon(&otherPlaneBelowIcon, 18, colorBelow);
    createIcon(&otherPlaneSameIcon, 18, colorSame);
    createIcon(&otherPlaneAboveIcon, 18, colorAbove);
}

void AircraftIcons::createIcon(img::Image *icon, int size, uint32_t color) {

    static const char *planeIconFormat = R"SVG(
<svg width="300" height="300" xmlns="http://www.w3.org/2000/svg">
<g transform="translate(150,150)" style="stroke:none;fill:rgb({},{},{})">
<ellipse rx="15" ry="25" cx="0" cy="-100" />
<rect width="30" height="190" x="-15" y="-100" />
<polygon points="0,-50 -110,30 -110,40 0,10 110,40 110, 30" />
<polygon points="0,50 -40,100 -40,110 0,100 40,110 40,100" />
</g></svg>
)SVG";

    // manufacture a new icon and put it in the cache
    auto planeIconSvg = fmt::format(planeIconFormat, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    auto document = lunasvg::Document::loadFromData(planeIconSvg);
    assert(document);
    auto bitmap = document->renderToBitmap(size, size, 0);
    assert(bitmap.valid());
    bitmap.convertToRGBA();
    auto p = bitmap.data();
    icon->resize(size, size, 0);
    icon->setPixels(p, size, size);
}

AircraftIcons *AircraftIcons::singleton = nullptr;

} // namespace maps

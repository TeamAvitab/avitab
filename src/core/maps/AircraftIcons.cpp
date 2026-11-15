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
:   colorBelow(b), colorSame(s), colorAbove(a), timestamp(0)
{
}

const img::Image *AircraftIcons::getUserPlaneIcon(int hdg) {
    auto k = makeCacheKey(hdg, true, false, false);
    auto i = iconCache.find(k);
    if (i != iconCache.end()) {
        auto& e = i->second;
        e->lastUsed = ++timestamp;
        return &(e->icon);
    }
    return createIcon(hdg, true, false, false, k);
}

const img::Image *AircraftIcons::getOtherPlaneIcon(int hdg, bool below, bool above) {
    auto k = makeCacheKey(hdg, false, below, above);
    auto i = iconCache.find(k);
    if (i != iconCache.end()) {
        auto& e = i->second;
        e->lastUsed = ++timestamp;
        return &(e->icon);
    }
    return createIcon(hdg, false, below, above, k);
}

const img::Image *AircraftIcons::createIcon(int hdg, bool user, bool below, bool above, uint32_t key) {

    // if the cache is getting too big, then clear out the stale entries
    int oldestTimestamp = timestamp;
    while (iconCache.size() >= 30) {
        for (auto& ce : iconCache) {
            oldestTimestamp = std::min(ce.second->lastUsed, oldestTimestamp);
        }
        auto tsDelta = timestamp - oldestTimestamp;
        if (tsDelta > 20) oldestTimestamp = timestamp - 20;
        for (auto ci = iconCache.begin(); ci != iconCache.end(); ) {
            if (ci->second->lastUsed <= oldestTimestamp) {
                ci = iconCache.erase(ci);
            } else {
                ++ci;
            }
        }
    }

    static const char *planeIconFormat = R"SVG(
<svg width="300" height="300" xmlns="http://www.w3.org/2000/svg">
<g transform="translate(150,150)">
<g transform="rotate({})" style="stroke:none;fill:rgb({},{},{})">
<ellipse rx="15" ry="25" cx="0" cy="-100" />
<rect width="30" height="190" x="-15" y="-100" />
<polygon points="0,-50 -110,30 -110,40 0,10 110,40 110, 30" />
<polygon points="0,50 -40,100 -40,110 0,100 40,110 40,100" />
</g></g></svg>
)SVG";

    // manufacture a new icon and put it in the cache
    auto ce = std::make_unique<IconCacheEntry>();
    ce->lastUsed = ++timestamp;
    const unsigned iconDimension = user ? 28 : 18;
    uint32_t color = img::COLOR_LIGHT_RED;
    if (!user) { color = below ? colorBelow : (above ? colorAbove : colorSame ); }
    auto planeIconSvg = fmt::format(planeIconFormat, roundedHeading(hdg), (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    auto document = lunasvg::Document::loadFromData(planeIconSvg);
    assert(document);
    auto bitmap = document->renderToBitmap(iconDimension, iconDimension, 0);
    assert(bitmap.valid());
    bitmap.convertToRGBA();
    auto p = bitmap.data();
    ce->icon.resize(iconDimension, iconDimension, 0);
    ce->icon.setPixels(p, iconDimension, iconDimension);
    img::Image *i = &(ce->icon);
    iconCache[key] = std::move(ce);
    return i;
}

uint32_t AircraftIcons::makeCacheKey(int hdg, bool user, bool below, bool above) {
    uint32_t k = roundedHeading(hdg);
    if (user) k += 1 << 9;
    if (below) k += 1 << 10;
    if (above) k += 1 << 11;
    return k;
}

int AircraftIcons::roundedHeading(int hdg) {
    // to avoid having too many icons in the cache with indistinguishably small
    // heading differences, the heading is rounded to the nearest multiple of 6 degrees.
    const int arcAngleMin = 6;
    return arcAngleMin * (((360 + hdg + (arcAngleMin / 2)) % 360) / arcAngleMin);
}

AircraftIcons *AircraftIcons::singleton = nullptr;

} // namespace maps

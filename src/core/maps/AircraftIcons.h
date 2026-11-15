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

#include "image/Image.h"
#include <cstdint>
#include <map>
#include <memory>

namespace maps {

// Provides aircraft icons for overlaying on maps and charts.
// These are drawn and cached as required to improve performance by avoiding
// rotation on each frame.

class AircraftIcons {
public:
    static AircraftIcons *factory(uint32_t colorBelow, uint32_t colorSame, uint32_t colorAbove);

    const img::Image *getUserPlaneIcon(int hdg);
    const img::Image *getOtherPlaneIcon(int hdg, bool below, bool above);

private:
    AircraftIcons(uint32_t colorBelow, uint32_t colorSame, uint32_t colorAbove);
    virtual ~AircraftIcons() = default;

    const img::Image *createIcon(int hdg, bool user, bool below, bool above, uint32_t key);

private:
    static AircraftIcons *singleton;
    const uint32_t colorBelow;
    const uint32_t colorSame;
    const uint32_t colorAbove;

private:
    // cached images
    struct IconCacheEntry {
        img::Image icon;
        int lastUsed;
    };
    std::map<uint32_t, std::unique_ptr<IconCacheEntry>> iconCache;
    int timestamp;

    uint32_t makeCacheKey(int hdg, bool user, bool below, bool above);
    int roundedHeading(int hdg);

};

}

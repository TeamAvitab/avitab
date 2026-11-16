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

    const img::Image *getUserPlaneIcon() { return &userPlaneIcon; }
    const img::Image *getOtherPlaneIcon(bool below, bool above) { return below ? &otherPlaneBelowIcon : (above ? &otherPlaneAboveIcon : &otherPlaneSameIcon ); }

private:
    AircraftIcons(uint32_t colorBelow, uint32_t colorSame, uint32_t colorAbove);
    virtual ~AircraftIcons() = default;

    void createIcon(img::Image *icon, int size, uint32_t color);

private:
    static AircraftIcons *singleton;
    const uint32_t colorBelow;
    const uint32_t colorSame;
    const uint32_t colorAbove;

    img::Image userPlaneIcon;
    img::Image otherPlaneBelowIcon;
    img::Image otherPlaneSameIcon;
    img::Image otherPlaneAboveIcon;
};

}

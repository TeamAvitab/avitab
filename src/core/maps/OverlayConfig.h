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

#include "Settings.h"
#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include <cstdint>

namespace maps {

struct OverlayConfig : public avitab::ConfigBase {
    OverlayConfig();
    void disable();
    bool drawMyAircraft = true;
    bool drawOtherAircraft = true;
    uint32_t colorOtherAircraftBelow;
    uint32_t colorOtherAircraftSame;
    uint32_t colorOtherAircraftAbove;
    bool drawAirports = false;
    bool drawAirstrips = false;
    bool drawHeliportsSeaports = false;
    bool drawVORs = false;
    bool drawNDBs = false;
    bool drawILSs = false;
    bool drawWaypoints = false;
    bool drawPOIs = false;
    bool drawVRPs = false;
    bool drawMarkers = false;
    bool drawRoute = true;
    void pack() override;
    void unpack() override;
private:
    uint32_t colorStringToInt(std::string colString);
    std::string colorIntToString(uint32_t color);
    static std::map<std::string, uint32_t> colorTable;
};

} /* namespace maps */

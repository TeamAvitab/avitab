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

#include "OverlayConfig.h"
#include <sstream>
#include <cassert>

namespace maps {

void OverlayConfig::pack() {
    json["my_aircraft"] = drawMyAircraft;
    json["other_aircraft"] = drawOtherAircraft;
    json["airports"]       = drawAirports;
    json["airstrips"]      = drawAirstrips;
    json["heliports_seaports"] = drawHeliportsSeaports;
    json["VORs"]               = drawVORs;
    json["NDBs"]               = drawNDBs;
    json["ILSs"]               = drawILSs;
    json["waypoints"]          = drawWaypoints;
    json["POIs"]               = drawPOIs;
    json["VRPs"]               = drawVRPs;
    json["markers"]            = drawMarkers;
    json["route"]              = drawRoute;
    json["colors"]["other_aircraft"]["below"] = colorIntToString(colorOtherAircraftBelow);
    json["colors"]["other_aircraft"]["same"] = colorIntToString(colorOtherAircraftSame);
    json["colors"]["other_aircraft"]["above"] = colorIntToString(colorOtherAircraftAbove);
}

void OverlayConfig::unpack() {
    drawMyAircraft = json.value("my_aircraft", true);
    drawOtherAircraft = json.value("other_aircraft", true);
    drawAirports = json.value("airports", false);
    drawAirstrips = json.value("airstrips", false);
    drawHeliportsSeaports = json.value("heliports_seaports", false);
    drawVORs = json.value("VORs", false);
    drawNDBs = json.value("NDBs", false);
    drawILSs = json.value("ILSs", false);
    drawWaypoints = json.value("waypoints", false);
    drawPOIs = json.value("POIs", false);
    drawVRPs = json.value("VRPs", false);
    drawMarkers = json.value("markers", false);
    drawRoute = json.value("route", true);
    nlohmann::json cols_json = nlohmann::json::object();
    auto jp = nlohmann::json::json_pointer("/colors/other_aircraft");
    if (json.contains(jp)) {
        cols_json = json[jp];
    }
    colorOtherAircraftBelow = colorStringToInt(cols_json.value("below", "GREEN"));
    colorOtherAircraftSame = colorStringToInt(cols_json.value("same", "BLACK"));
    colorOtherAircraftAbove = colorStringToInt(cols_json.value("above", "BLUE"));
}

OverlayConfig::OverlayConfig() {
    if (colorTable.empty()) {
        colorTable["BLACK"] =  0xFF000000;
        colorTable["GREY"] =   0xFF303030;
        colorTable["WHITE"] =  0xFFFFFFFF;
        colorTable["RED"] =    0xFF800000;
        colorTable["BLUE"] =   0xFF0000A0;
        colorTable["GREEN"] =  0xFF008000;
        colorTable["YELLOW"] = 0xFF808000;
    }
}

void OverlayConfig::disable() {
    drawMyAircraft = false;
    drawOtherAircraft = false;
    drawAirports = false;
    drawAirstrips = false;
    drawHeliportsSeaports = false;
    drawVORs = false;
    drawNDBs = false;
    drawILSs = false;
    drawWaypoints = false;
    drawPOIs = false;
    drawVRPs = false;
    drawMarkers = false;
    drawRoute = false;
}

uint32_t OverlayConfig::colorStringToInt(std::string colString)
{
    std::string cstr(colString);
    for (auto i = cstr.begin(); i != cstr.end(); ++i) *i = toupper(*i);

    if (colorTable.find(cstr) != colorTable.end()) {
        return colorTable[cstr];
    }

    if ((cstr[0] == '#') && (cstr.size() >= 7)) {
        cstr.erase(0,1);
        uint32_t c = 0;
        size_t p = 0;
        try {
            long long l = std::stoll(cstr, &p, 16);
            c = static_cast<uint32_t>(static_cast<unsigned long long>(l) & 0xFFFFFFFF);
        } catch (...) {
            p = 0;
        }
        if (p == 6) c |= 0xFF000000;// make color opaque if alpha-channel not defined
        if (p) {
            colorTable[colString] = c;
            return c;
        }
    }

    return 0xFFFFFFFF;
}

std::string OverlayConfig::colorIntToString(uint32_t color)
{
    for (auto i = colorTable.begin(); i != colorTable.end(); ++i) {
        if (color == i->second) {
            return std::string(i->first);
        }
    }
    std::stringstream s;
    s << "#" << std::setfill('0') << std::setw(8) << std::hex << color;
    return s.str();
}

std::map<std::string, uint32_t> OverlayConfig::colorTable;

}

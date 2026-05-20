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
#include <cmath>
#include <ctime>
#include "StandAloneEnvironment.h"
#include "Logger.h"
#include "platform/Platform.h"

StandAloneEnvironment::StandAloneEnvironment() : ToolEnvironment() {
}

void StandAloneEnvironment::eventLoop() {
    while (driver->handleEvents()) {
        runEnvironmentCallbacks();
        setLastFrameTime(driver->getLastDrawTime() / 1000.0);
    }
    driver.reset();
}

std::shared_ptr<avitab::GUIDriver> StandAloneEnvironment::createGUIDriver() {
    driver = std::make_shared<GlfwGUIDriver>();
    return driver;
}

std::vector<float> StandAloneEnvironment::getMagneticVariations(std::vector<world::Location> &locs) {
    std::vector<float> mvs;
    for (auto location : locs) {
        mvs.push_back(((float)std::rand() / RAND_MAX - 0.5) * 10);
    }
    return mvs;
}

std::string StandAloneEnvironment::getMETARForAirport(const std::string &icao) {
    return "";
}

int StandAloneEnvironment::getWeatherAtLocation(const world::Location &loc, const float &altitude, std::string &weather) {
    weather = std::string("Wind calm, Visibility 7 nm, Temp./Dew 14/8 °C, QNH 1013");
    return 1;
}

std::string StandAloneEnvironment::getNearestAirportId() {
    return "EDHL";
}

avitab::AircraftID StandAloneEnvironment::getActiveAircraftCount() {
    return 4;
}

world::Position StandAloneEnvironment::getAircraftPosition(avitab::AircraftID id) {
    static unsigned int t = 0;
    static world::Position loc[4];
    static double vel[4];
    static double trn[4];
    static double asc[4];
    if (t == 0) {
        // various options for the user aircraft
#if 0 // normal setting, user aircraft at EDHL, no lateral movement
        loc[0] = world::Position::fromGCSm(53.8019434, 10.7017287, 70, 400);
        vel[0] = 0.0;
        trn[0] = 7;
        asc[0] = 2;
#elif 1 // alternative setting, user aircraft circling EDHL
        loc[0] = world::Position::fromGCSm(53.81, 10.735, 340, 400);
        vel[0] = 0.005;
        trn[0] = -4;
        asc[0] = 2;
#else // these settings can be enabed for reviewing map tiling behaviour at the
      // 180/-180 boundary
        loc[0] = world::Position::fromGCSm(67.8, 179.8, 20, 400);
        vel[0] = 0.018;
        trn[0] = 2;
        asc[0] = 2;
#endif
        // routing of other aircraft
        loc[1] = world::Position::fromGCSm(53.81, 10.69, 230, 400);
        trn[1] = 8;
        vel[1] = 0.007;
        asc[1] = 3;
        loc[2] = world::Position::fromGCSm(53.79, 10.7, 2, 200);
        trn[2] = -5;
        vel[2] = 0.004;
        asc[2] = 5;
        loc[3] = world::Position::fromGCSm(53.82, 10.74, 100, 5000);
        trn[3] = 7;
        vel[3] = 0.013;
        asc[3] = -8;
    } else {
        for (size_t i = 0; i < 4; ++i) {
            auto newpos = loc[i].greatCircleWaypoint(vel[i] * world::NM_TO_RAD);
            double hdg = newpos.hdg_rad + (trn[i] * world::DEG_TO_RAD / 10);
            double alt = loc[i].alt_metres + asc[i];
            if (alt < 30) { asc[i] = std::fabs(asc[i]); }
            if (alt > 5000) { asc[i] = 0.0 - std::fabs(asc[i]); }
            loc[i] = world::Position(world::Trajectory(world::Location(newpos.ypos_rad, newpos.xpos_rad), hdg), alt);
        }
    }
    ++t;
    return loc[id];
}

unsigned int StandAloneEnvironment::getZuluTimeSeconds() {
    time_t now = time(nullptr);
    tm *zulu = gmtime(&now);
    return zulu->tm_hour * 3600 + zulu->tm_min * 60 + zulu->tm_sec;
}

unsigned int StandAloneEnvironment::getLocalTimeSeconds() {
    time_t now = time(nullptr);
    tm *local = localtime(&now);
    return local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;
}

StandAloneEnvironment::~StandAloneEnvironment() {
    logger::verbose("~StandAloneEnvironment");
}

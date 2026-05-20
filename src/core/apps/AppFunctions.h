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

#include <filesystem>
#include <memory>
#include <string>
#include "gui/widgets/Container.h"
#include "Navigation.h"
#include "charts/ChartService.h"
#include "Environment.h"

namespace navdb {
class Route;
}

namespace avitab {

enum class AppId {
    CHARTS,
    AIRPORTS,
    ROUTES,
    MAPS,
    PLANE_MANUAL,
    NOTES,
    NAVIGRAPH,
    ABOUT,
};

class AppFunctions {
    // REFACTOR - some of these APIs are simply forwarded to the environment.
    // they could be removed by passing the environment to apps on creation.
    // also are they really needed in an abstract API, or could they simply be called directly?
public:
    virtual void setBrightness(float brightness) = 0;
    virtual float getBrightness() = 0;
    virtual void executeLater(std::function<void()> func) = 0;
    virtual std::filesystem::path getAvitabInstallDir() = 0;
    virtual std::filesystem::path getAvitabDataDir() = 0;
    virtual std::filesystem::path getAirplanePath() = 0;
    virtual std::filesystem::path getFlightPlansPath() = 0;
    virtual std::shared_ptr<Container> createGUIContainer() = 0;
    virtual void showGUIContainer(std::shared_ptr<Container> container) = 0;
    virtual void onHomeButton() = 0;
    virtual std::shared_ptr<navdb::NavDatabase> getNavDatabase() = 0;
    virtual void loadUserFixes(const std::filesystem::path &filename) = 0;
    virtual std::vector<float> getMagneticVariations(std::vector<world::Location> &locs) = 0;
    virtual std::string getMETARForAirport(const std::string &icao) = 0;
    virtual int getWeatherAtLocation(const world::Location &loc, const float &altitude, std::string &weather) = 0;
    virtual std::string getNearestAirportId() = 0;
    virtual void close() = 0;
    virtual void setIsInMenu(bool inMenu) = 0;
    virtual std::shared_ptr<apis::ChartService> getChartService() = 0;
    virtual unsigned int getActiveAircraftCount() = 0;
    virtual world::Position getAircraftPosition(AircraftID id) = 0;
    virtual float getLastFrameTime() = 0;
    virtual unsigned int getZuluTimeSeconds() = 0;
    virtual unsigned int getLocalTimeSeconds() = 0;
    virtual std::shared_ptr<Settings> getSettings() = 0;

    virtual void setRoute(std::shared_ptr<navdb::Route> route) = 0;
    virtual std::shared_ptr<navdb::Route> getRoute() = 0;
    
    virtual void updateMapExports(float lat, float lon, int zoom, float vrange) = 0;
    virtual ~AppFunctions() = default;
};

}

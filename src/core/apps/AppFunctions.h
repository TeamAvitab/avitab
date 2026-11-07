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
#include "gui/Theme.h"
#include "Navigation.h"
#include "charts/ChartService.h"
#include "SimDriver.h"
#include "Settings.h"

namespace navdb {
class Route;
}

namespace avitab {

enum class AppId {
    CHARTS,
    AIRPORTS,
    ROUTES,
    MAPS,
    MANUALS,
    NOTES,
    PROVIDERS,
    ABOUT,
};

class AppFunctions {
    // REFACTOR - some of these APIs are simply forwarded to the simDriver.
    // they could be removed by passing the simDriver to apps on creation.
    // also are they really needed in an abstract API, or could they simply be called directly?
public:
    virtual ~AppFunctions() = default;

    // APIs related to filesystem paths
    virtual std::filesystem::path getAvitabInstallDir() = 0;
    virtual std::filesystem::path getAvitabDataDir() = 0;
    virtual std::filesystem::path getAvitabThemeDir() = 0;
    virtual std::filesystem::path getAirplanePath() = 0;
    virtual std::filesystem::path getFlightPlansPath() = 0;

    // APIs related to other service
    virtual std::shared_ptr<Settings> getSettings() = 0;
    virtual std::shared_ptr<apis::ChartService> getChartService() = 0;
    virtual std::shared_ptr<navdb::NavDatabase> getNavDatabase() = 0;

    // APIs managing high-level app navigation - REFACTOR: rename these
    virtual void onHomeButtonPressed() = 0;
    virtual void onAppClosed() = 0;
    virtual void notifyHomeScreenActive(bool inMenu) = 0;
    virtual void notifyMapConfiguration(float lat, float lon, int zoom, float vrange) = 0;

    // APIs related to NAV data services (REFACTOR - wrap these directly into NavDatabase?)
    virtual void loadUserFixes(const std::filesystem::path &filename) = 0;
    virtual std::string getNearestAirportId() = 0;
    virtual std::vector<float> getMagneticVariations(std::vector<world::Location> &locs) = 0;
    virtual void setRoute(std::shared_ptr<navdb::Route> route) = 0;
    virtual std::shared_ptr<navdb::Route> getRoute() = 0;

    // APIs related to METAR and weather
    virtual std::string getMETARForAirport(const std::string &icao) = 0;
    virtual int getWeatherAtLocation(const world::Location &loc, const float &altitude, std::string &weather) = 0;

    // APIs related to other aircraft traffic
    virtual unsigned int getActiveAircraftCount() = 0;
    virtual world::Position getAircraftPosition(AircraftID id) = 0;

    // APIs related to sim time and timing
    virtual unsigned int getFramesPerSecond() = 0;
    virtual unsigned int getZuluTimeSeconds() = 0;
    virtual unsigned int getLocalTimeSeconds() = 0;

    // APIs related to UI control
    virtual void setBrightness(float brightness) = 0;
    virtual float getBrightness() = 0;

    // Schedule jobs for deferred execution
    virtual void executeLater(std::function<void()> func) = 0;

    // APIs for LVGL widget stuff (REFACTOR put these into a separate interface and implement it in LVGL toolkit)
    virtual std::shared_ptr<Theme> getGUITheme() = 0;
    virtual std::shared_ptr<Container> createGUIContainer() = 0;
    virtual void showGUIContainer(std::shared_ptr<Container> container) = 0;
};

}

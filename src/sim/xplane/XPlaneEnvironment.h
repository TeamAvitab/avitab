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

#include <XPLM/XPLMMenus.h>
#include <XPLM/XPLMUtilities.h>
#include <XPLM/XPLMProcessing.h>
#include <XPLM/XPLMWeather.h>
#include <memory>
#include <vector>
#include <atomic>
#include <map>
#include <thread>
#include "Environment.h"
#include "WorldGeometry.h"
#include "DataCache.h"
#include "DataRefExport.h"

class XPlaneEnvironment: public avitab::Environment {
public:
    XPlaneEnvironment();

    // Must be called from the environment thread - do not call from GUI thread!
    std::shared_ptr<avitab::GUIDriver> createGUIDriver() override;
    void createMenu(const std::string &name) override;
    void addMenuEntry(const std::string &label, MenuCallback cb) override;
    void destroyMenu() override;
    void createCommand(const std::string &name, const std::string &desc, CommandCallback cb) override;
    void destroyCommands() override;
    void onAircraftReload() override;

    // Can be called from any thread
    std::filesystem::path getProgramPath() override;
    std::filesystem::path getDataRootPath() override;
    std::filesystem::path getSettingsDir() override;
    std::filesystem::path getAirplanePath() override;
    std::filesystem::path getFontDirectory() override;
    std::filesystem::path getFlightPlansPath() override;
    std::filesystem::path getXpNavDataRootPath() override;
    std::filesystem::path getMsfsNavDataRootPath() override { return ""; }

    std::vector<float> getMagneticVariations(std::vector<world::Location> &locs) override;
    std::string getMETARForAirport(const std::string &icao) override;
    int getWeatherAtLocation(const world::Location &loc, const float &altitude, std::string& weather) override;
    std::string getNearestAirportId() override;
    void enableAndPowerPanel() override;
    void setIsInMenu(bool menu) override;
    avitab::AircraftID getActiveAircraftCount() override;
    world::Position getAircraftPosition(avitab::AircraftID id) override;
    void updateMapExports(float lat, float lon, int zoom, float vrange) override;
    unsigned int getZuluTimeSeconds() override;
    unsigned int getLocalTimeSeconds() override;

    ~XPlaneEnvironment();

private:
    // Exported datarefs relating to the overlayed map status
    float getMapLatitude();
    float getMapLongitude();
    int getMapZoom();
    float getMapVerticalRange();
    std::unique_ptr<xdata::DataRefExport<float>> mapLatitudeRef;
    std::unique_ptr<xdata::DataRefExport<float>> mapLongitudeRef;
    std::unique_ptr<xdata::DataRefExport<int>> mapZoomRef;
    std::unique_ptr<xdata::DataRefExport<float>> mapVerticalRangeRef;

private:
    using GetMetarPtr = void(*)(const char *id, XPLMFixedString150_t *outMETAR);
    using GetWeatherPtr = int(*)(double latitude, double longitude, double altitude, XPLMWeatherInfo_t *weather);

    struct RegisteredCommand {
        CommandCallback callback;
        bool inBefore;
        void *refCon;
    };

    // Cached data
    GetMetarPtr getMetar{};
    GetWeatherPtr getWeatherAtLoc{};
    xdata::DataCache dataCache;
    std::filesystem::path pluginPath, xplanePrefsDir, xplaneRootPath;
    int xplaneVersion;
    world::Position nullPosition = world::Position(world::Trajectory(world::Location(0, 0), 0), 0);

private:
    // State updated/accessed by multiple threads, mutex protected
    std::mutex stateMutex;
    std::filesystem::path aircraftPath;
    std::vector<world::Position> aircraftLocations;
    unsigned int otherAircraftCount;
    float mapLatitude { 0.0f };
    float mapLongitude { 0.0f };
    int mapZoom { 0 };
    float mapVerticalRange { 0.0f };
    unsigned int zuluTimeSecs;
    unsigned int localTimeSecs;
    std::atomic<float> lastFrameTime {};

private:
    std::vector<MenuCallback> menuCallbacks;
    std::atomic<XPLMFlightLoopID> flightLoopId { nullptr };
    std::map<XPLMCommandRef, RegisteredCommand> commandHandlers;
    int subMenuIdx = -1;
    XPLMMenuID subMenu = nullptr;
    std::shared_ptr<int> panelPowered, panelEnabled;
    std::shared_ptr<float> brightness;
    std::unique_ptr<xdata::DataRefExport<int>> panelPoweredRef, panelEnabledRef, isInMenuRef;
    std::unique_ptr<xdata::DataRefExport<float>> brightnessRef;

    bool isInMenu = false;

    std::filesystem::path getXPlanePath();
    std::filesystem::path getPluginPath();
    std::filesystem::path findPreferencesDir();
    XPLMFlightLoopID createFlightLoop();
    float onFlightLoop(float elapsedSinceLastCall, float elapseSinceLastLoop, int count);
    static int handleCommand(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref);
    xdata::EnvData getData(const std::string &dataRef);
    void reloadAircraftPath();

    enum cloudCoverage {
        SKY_CLEAR     = 5,
        SKY_FEW       = 25,
        SKY_SCATTERED = 60,
        SKY_BROKEN    = 80,
        SKY_OVERCAST  = 100,
    };
    std::string cloudCoverageToText(float coverage);

    // ============================================================
    // New TCAS AI/multiplayer interface
    // int integer If TCAS is not overriden by plugin, returns the number of planes in X-Plane, which might be under plugin control or X-Plane control. If TCAS is overriden, returns how many targets are actually being written to with the override. These are not necessarily consecutive entries in the TCAS arrays.
    unsigned int tcasAircraftCount;

    // int[64] integer Mode C transponder code 0000 to 7777. This is not really an integer, this is an octal number.
    std::vector<int> tcasModeCcode;
    // float[64] degrees global coordinate, degrees.
    std::vector<float> tcasLat;
    // float[64] degrees global coordinate, degrees.
    std::vector<float> tcasLon;
    // float[64] meter global coordinate, meter.
    std::vector<float> tcasEle;
    // float[64] degrees true heading orientation.
    std::vector<float> tcasPsi;

    void updatePlaneCount();
};

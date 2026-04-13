/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2025 Folke Will
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
//#include "core/gui_toolkit/LVGLToolkit.h"
#include "core/env/Environment.h"
#include "DataCache.h"
#include "DataRefExport.h"

namespace avitab {

class XPlaneEnvironment: public Environment {
public:
    XPlaneEnvironment();

    // Must be called from the environment thread - do not call from GUI thread!
    std::shared_ptr<world::LoadManager> createParsingWorldManager() override;
    std::shared_ptr<LVGLToolkit> createGUIToolkit() override;
    void createMenu(const std::string &name) override;
    void addMenuEntry(const std::string &label, MenuCallback cb) override;
    void destroyMenu() override;
    void createCommand(const std::string &name, const std::string &desc, CommandCallback cb) override;
    void destroyCommands() override;
    void onAircraftReload() override;

    // Can be called from any thread
    std::string getFontDirectory() override;
    std::string getProgramPath() override;
    std::string getDataRootPath() override;
    std::string getSettingsDir() override;
    std::string getEarthTexturePath() override;
    std::string getAirplanePath() override;
    std::string getFlightPlansPath() override;
    Environment::MagVarMap getMagneticVariations(std::vector<std::pair<double, double>> locations) override;
    std::string getMETARForAirport(const std::string &icao) override;
    int getWeatherAtLocation(const world::Location &loc, const float &altitude, std::shared_ptr<std::string> &weather) override;
    std::string getNearestAirportId() override;
    void enableAndPowerPanel() override;
    void setIsInMenu(bool menu) override;
    AircraftID getActiveAircraftCount() override;
    Location getAircraftLocation(AircraftID id) override;
    void updateMapExports(float lat, float lon, int zoom, float vrange) override;
    unsigned int getZuluTimeSeconds() override;
    unsigned int getLocalTimeSeconds() override;

    ~XPlaneEnvironment();

protected:
    bool canUseNavDb(const std::string simCode) override;

private:
    // Exported datarefs relating to the overlayed map status
    float getMapLatitude();
    float getMapLongitude();
    int getMapZoom();
    float getMapVerticalRange();
    std::unique_ptr<DataRefExport<float>> mapLatitudeRef;
    std::unique_ptr<DataRefExport<float>> mapLongitudeRef;
    std::unique_ptr<DataRefExport<int>> mapZoomRef;
    std::unique_ptr<DataRefExport<float>> mapVerticalRangeRef;

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
    DataCache dataCache;
    std::string pluginPath, xplanePrefsDir, xplaneRootPath;
    int xplaneVersion;
    Location nullLocation { 0, 0, 0, 0 };

private:
    // State updated/accessed by multiple threads, mutex protected
    std::mutex stateMutex;
    std::string aircraftPath;
    std::vector<Location> aircraftLocations;
    unsigned int otherAircraftCount;
    float mapLatitude { 0.0f };
    float mapLongitude { 0.0f };
    int mapZoom { 0 };
    float mapVerticalRange { 0.0f };
    unsigned int zuluTimeSecs;
    unsigned int localTimeSecs;

private:
    std::vector<MenuCallback> menuCallbacks;
    std::atomic<XPLMFlightLoopID> flightLoopId { nullptr };
    std::map<XPLMCommandRef, RegisteredCommand> commandHandlers;
    int subMenuIdx = -1;
    XPLMMenuID subMenu = nullptr;
    std::shared_ptr<int> panelPowered, panelEnabled;
    std::shared_ptr<float> brightness;
    std::unique_ptr<DataRefExport<int>> panelPoweredRef, panelEnabledRef, isInMenuRef;
    std::unique_ptr<DataRefExport<float>> brightnessRef;

    bool isInMenu = false;

    std::string getXPlanePath();
    std::string getPluginPath();
    std::string findPreferencesDir();
    XPLMFlightLoopID createFlightLoop();
    float onFlightLoop(float elapsedSinceLastCall, float elapseSinceLastLoop, int count);
    static int handleCommand(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref);
    EnvData getData(const std::string &dataRef);
    void reloadAircraftPath();

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

} /* namespace avitab */

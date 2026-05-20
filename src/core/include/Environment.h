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

#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include <vector>
#include <future>
#include <atomic>
#include <filesystem>
#include "JsonConfig.h"
#include "Settings.h"
#include "WorldGeometry.h"
#include "Navigation.h"

namespace avitab {

using AircraftID = unsigned int;

enum class CommandState {
    START,
    CONTINUE,
    END
};

/**
 * This interface defines methods to interact with the environment
 * of the application, e.g. X-Plane or the stand-alone variant.
 */

// REFACTOR - there's stuff in here that's common to all environments.
// In that case, surely it should just go in the Avitab core?
// This should eventually be an abstract interface.

class Environment {
public:
    using MenuCallback = std::function<void()>;
    using CommandCallback = std::function<void(CommandState)>;
    using EnvironmentCallback = std::function<void()>;

    // Must be called from the environment thread - do not call from GUI thread!


    // REFACTOR - obsolete? combine with settings?
    void loadConfig();
    std::shared_ptr<JsonConfig> getConfig();

    // REFACTOR - move into core, nothing to do with the environment
    void loadSettings();
    std::shared_ptr<Settings> getSettings();

    virtual void onAircraftReload() = 0;
    virtual std::shared_ptr<GUIDriver> createGUIDriver() = 0;

    // REFACTOR - move into X-Plane environment, not core
    virtual void createMenu(const std::string &name) = 0;
    virtual void addMenuEntry(const std::string &label, MenuCallback cb) = 0;
    virtual void destroyMenu() = 0;
    virtual void createCommand(const std::string &name, const std::string &desc, CommandCallback cb) = 0;
    virtual void destroyCommands() = 0;
    virtual void enableAndPowerPanel() = 0;

    // Can be called from any thread

    void pauseEnvironmentJobs();
    void resumeEnvironmentJobs();


    /**
     * Stores a callback in the pending callbacks queue to
     * be executed by the environment thread.
     * @param cb the callback to enqueue
     */
    void runInEnvironment(EnvironmentCallback cb);

    virtual std::filesystem::path getProgramPath() = 0;
    virtual std::filesystem::path getDataRootPath() = 0;
    virtual std::filesystem::path getSettingsDir() = 0;
    virtual std::filesystem::path getFlightPlansPath() = 0;
    virtual std::filesystem::path getFontDirectory() = 0;
    virtual std::filesystem::path getAirplanePath() = 0;
    virtual std::filesystem::path getXpNavDataRootPath() = 0;
    virtual std::filesystem::path getMsfsNavDataRootPath() = 0;

    // Getting magVar from XPlane is asynchronous and slow, so batch request
    virtual std::vector<float> getMagneticVariations(std::vector<world::Location> &locs) = 0;

    virtual std::string getMETARForAirport(const std::string &icao) = 0;
    virtual int getWeatherAtLocation(const world::Location &loc, const float &altitude, std::string &weather) = 0;

    // REFACTOR - should be part of the navdb::NavDatabase interface
    virtual std::string getNearestAirportId() = 0;

    // these relate to exporting Avitab internal state to the sim - only relevant to X-Plane.
    // we might want to consider a publish/subscribe pattern instead?
    virtual void setIsInMenu(bool menu) = 0;
    virtual void updateMapExports(float lat, float lon, int zoom, float vrange) = 0;

    virtual AircraftID getActiveAircraftCount() = 0;
    virtual world::Position getAircraftPosition(AircraftID id) = 0;
    virtual unsigned int getZuluTimeSeconds() = 0;
    virtual unsigned int getLocalTimeSeconds() = 0;


    virtual ~Environment() = default;

public:
    float getLastFrameTime();

protected:
    void runEnvironmentCallbacks();
    void setLastFrameTime(float t);

private:
    std::shared_ptr<JsonConfig> config;
    std::shared_ptr<Settings> settings;
    std::mutex envMutex;
    std::vector<EnvironmentCallback> envCallbacks;
    bool stopped = false;
    std::atomic<float> lastFrameTime {};
};

} /* namespace avitab */

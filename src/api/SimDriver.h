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
#include <array>
#include <vector>
#include <future>
#include <atomic>
#include <filesystem>
#include "WorldGeometry.h"
#include "Navigation.h"

namespace avitab {

using AircraftID = unsigned int;

class Settings;
class AviTabCore;

// The SimDriverBase interface defines methods for the Avitab core to interact with a simulation,
// which may be X-Plane, MSFS, or the mock simulator (used for desktop testing).

class SimDriverBase {
public:
    SimDriverBase();
    virtual ~SimDriverBase() = default;

    // Can be called from any thread
    virtual std::filesystem::path getProgramPath() = 0;
    virtual std::filesystem::path getDataRootPath() = 0;
    virtual std::filesystem::path getSettingsDir() = 0;
    virtual std::filesystem::path getFlightPlansPath() = 0;
    virtual std::filesystem::path getFontDirectory() = 0;
    virtual std::filesystem::path getThemeDirectory() = 0;
    virtual std::filesystem::path getAirplanePath() = 0;
    virtual std::filesystem::path getXpNavDataRootPath() = 0;
    virtual std::filesystem::path getMsfsNavDataRootPath() = 0;

    // Getting magVar from XPlane is asynchronous and slow, so batch request
    virtual std::vector<float> getMagneticVariations(std::vector<world::Location> &locs) = 0;

    // METAR/weather APIs
    virtual std::string getMETARForAirport(const std::string &icao) = 0;
    virtual int getWeatherAtLocation(const world::Location &loc, const float &altitude, std::string &weather) = 0;

    // REFACTOR - should be part of the navdb::NavDatabase interface
    virtual std::string getNearestAirportId() = 0;

    // these relate to exporting Avitab internal state to the sim - only relevant to X-Plane.
    // we might want to consider a publish/subscribe pattern instead?
    virtual void notifyIsInMenu(bool menu) = 0;
    virtual void notifyMapConfiguration(float lat, float lon, int zoom, float vrange) = 0;

    // Other aircraft APIs
    virtual AircraftID getActiveAircraftCount() = 0;
    virtual world::Position getAircraftPosition(AircraftID id) = 0;

    // Time and performance APIs
    virtual unsigned int getZuluTimeSeconds() = 0;
    virtual unsigned int getLocalTimeSeconds() = 0;
    unsigned int getFramesPerSecond();

    // Job running is implemented in this base class
    using SimDriverCallback = std::function<void()>;
    void runInSimDriver(SimDriverCallback cb);

public:
    // APIs used by the product wrappers
    void runSimDriverCallbacks();
    void notifyFrameDuration(unsigned int tMs);

protected:
    void resumeSimDriverJobs();
    void pauseSimDriverJobs();

private:
    std::mutex lockMutex;
    std::vector<SimDriverCallback> simDriverCallbacks;
    bool stopped = false;

    static constexpr size_t RING_BUFFER_SIZE = 20;
    std::array<unsigned int, RING_BUFFER_SIZE> frameDurations = {};
    size_t nextSlot = 0;
};

} /* namespace avitab */

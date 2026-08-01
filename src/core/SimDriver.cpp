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


#include "SimDriver.h"
#include "Logger.h"
#include "platform/CrashHandler.h"

namespace avitab {

void SimDriverBase::loadConfig() {
    config = std::make_unique<JsonConfig>(getDataRootPath() / "config.json",
                            R"({ "AviTab": { "logToStdOut": false, "loadNavData": true } })");
}

std::shared_ptr<JsonConfig> SimDriverBase::getConfig() {
    return config;
}

void SimDriverBase::loadSettings() {
    auto fname(getSettingsDir() / "avitab.prf");
    logger::info("Settings file: %s", fname.string().c_str());
    settings = std::make_unique<Settings>(fname);
}

std::shared_ptr<Settings> SimDriverBase::getSettings() {
    return settings;
}

void SimDriverBase::resumeSimDriverJobs() {
    std::lock_guard<std::mutex> lock(lockMutex);
    stopped = false;
}

void SimDriverBase::runInSimDriver(SimDriverCallback cb) {
    std::lock_guard<std::mutex> lock(lockMutex);
    if (stopped) {
        throw std::runtime_error("SimDriver is stopped");
    }
    simDriverCallbacks.push_back(cb);
}

void SimDriverBase::runSimDriverCallbacks() {
    std::lock_guard<std::mutex> lock(lockMutex);
    if (!simDriverCallbacks.empty()) {
        for (auto &cb: simDriverCallbacks) {
            cb();
        }
        simDriverCallbacks.clear();
    }
}

void SimDriverBase::pauseSimDriverJobs() {
    std::lock_guard<std::mutex> lock(lockMutex);
    stopped = true;
    for (auto &cb: simDriverCallbacks) {
        cb();
    }
    simDriverCallbacks.clear();
}

unsigned int SimDriverBase::getFramesPerSecond() {
    unsigned t = 0;
    {
        std::lock_guard<std::mutex> lock(lockMutex);
        for (auto i: frameDurations) {
            if (i == 0) return 0; // no report until ring buffer has been filled
            t += i;
        }
    }
    return (RING_BUFFER_SIZE * 1000) / t;
}

void SimDriverBase::reportFrameDuration(unsigned int tMs) {
    std::lock_guard<std::mutex> lock(lockMutex);
    frameDurations[nextSlot++] = tMs;
    nextSlot %= RING_BUFFER_SIZE;
}

}

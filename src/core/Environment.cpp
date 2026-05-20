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


#include "Environment.h"
#include "Logger.h"
#include "platform/CrashHandler.h"

namespace avitab {

void Environment::loadConfig() {
    config = std::make_unique<JsonConfig>(getDataRootPath() / "config.json",
                            R"({ "AviTab": { "logToStdOut": false, "loadNavData": true } })");
}

std::shared_ptr<JsonConfig> Environment::getConfig() {
    return config;
}

void Environment::loadSettings() {
    auto fname(getSettingsDir() / "avitab.prf");
    logger::info("Settings file: %s", fname.string().c_str());
    settings = std::make_unique<Settings>(fname);
}

std::shared_ptr<Settings> Environment::getSettings() {
    return settings;
}

void Environment::setLastFrameTime(float t) {
    lastFrameTime = t;
}

float Environment::getLastFrameTime() { return lastFrameTime; }

void Environment::resumeEnvironmentJobs() {
    std::lock_guard<std::mutex> lock(envMutex);
    stopped = false;
}

void Environment::runInEnvironment(EnvironmentCallback cb) {
    std::lock_guard<std::mutex> lock(envMutex);
    if (stopped) {
        throw std::runtime_error("Environment is stopped");
    }
    envCallbacks.push_back(cb);
}

void Environment::runEnvironmentCallbacks() {
    std::lock_guard<std::mutex> lock(envMutex);
    if (!envCallbacks.empty()) {
        for (auto &cb: envCallbacks) {
            cb();
        }
        envCallbacks.clear();
    }
}

void Environment::pauseEnvironmentJobs() {
    std::lock_guard<std::mutex> lock(envMutex);
    stopped = true;
    for (auto &cb: envCallbacks) {
        cb();
    }
    envCallbacks.clear();
}

}

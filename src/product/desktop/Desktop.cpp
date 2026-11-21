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
#include <memory>
#include <thread>
#include <iostream>
#include "Settings.h"
#include "sim/mock/MockSimDriver.h"
#include "ui/glfw/GlfwUiDriver.h"
#include "AviTabCore.h"
#include "Logger.h"
#include "platform/CrashHandler.h"

int main() {
    crash::registerHandler([] () {return 0;});

    try {
        // Using the heap so we can debug destructors with log messages
        auto simDriver = std::make_shared<MockSimDriver>();

        logger::setStdOut(true);
        logger::init(simDriver->getDataRootPath());
        logger::verbose("Main thread has id %d", std::this_thread::get_id());

        auto fname(simDriver->getSettingsDir() / "avitab.prf");
        auto settings = std::make_shared<avitab::Settings>(fname);

        auto uiDriver = std::make_shared<GlfwUiDriver>();

        auto aviTab = avitab::AviTabCore::CreateAviTabCore();

        // emulate plugin state machine
        simDriver->start(settings, aviTab);
        uiDriver->start(settings, aviTab);
        aviTab->start(settings, simDriver, uiDriver);

        simDriver->enable();
        uiDriver->enable();
        aviTab->enable();

        // desktop variant supports using the LVGL close window button to test
        // the disable -> enable lifecycle in the AviTab core.
        bool coreEnabled = true;
        bool disableRequested;
        auto disableTime = std::chrono::high_resolution_clock::now();

        // loops until window closed
        while (uiDriver->handleEvents(disableRequested)) {
            simDriver->runSimDriverCallbacks();
            simDriver->notifyFrameDuration(uiDriver->getLastDrawTime());

            if (coreEnabled) {
                if (disableRequested) {
                    aviTab->disable();
                    coreEnabled = false;
                    disableTime = std::chrono::high_resolution_clock::now();
                }
            } else {
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = now - disableTime;
                if (elapsed > std::chrono::milliseconds(5000)) {
                    aviTab->enable();
                    coreEnabled = true;
                }
            }
        }

        aviTab->disable();
        uiDriver->disable();
        simDriver->disable();

        aviTab->stop();
        uiDriver->stop();
        simDriver->stop();

        aviTab.reset();
        uiDriver.reset();
        simDriver.reset();
    } catch (const std::exception &e) {
        logger::error("Exception: %s", e.what());
    }

    logger::verbose("Quitting main");

    crash::unregisterHandler();

    return 0;
}

#ifdef _WIN32
#include <windows.h>

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main();
}
#endif

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
#include "sim/mock/MockSimDriver.h"
#include "AviTabCore.h"
#include "Logger.h"
#include "platform/CrashHandler.h"

int main() {
    crash::registerHandler([] () {return 0;});

    try {
        // Using the heap so we can debug destructors with log messages
        auto simDriver = std::make_shared<MockSimDriver>();
        try {
            simDriver->loadConfig();
        } catch (const std::exception &e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            exit(1);
        }
        logger::setStdOut(simDriver->getConfig()->getBool("/AviTab/logToStdOut"));
        logger::init(simDriver->getDataRootPath());
        logger::verbose("Main thread has id %d", std::this_thread::get_id());
        simDriver->loadSettings();

        auto uiDriver = simDriver->createUiDriver();
        auto aviTab = avitab::AviTabCore::CreateAviTabCore(simDriver, uiDriver);
        aviTab->startApp();
        aviTab->toggleTablet();

        // pauses until window closed
        simDriver->eventLoop();

        aviTab->stopApp();
        aviTab.reset();
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

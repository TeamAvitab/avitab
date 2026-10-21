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

#include <curl/curl.h>
#include <XPLM/XPLMDefs.h>
#include <XPLM/XPLMPlugin.h>
#include <memory>
#include <cstring>
#include "Settings.h"
#include "sim/xplane/XPlaneSimDriver.h"
#include "ui/xplane/XPlaneUiDriver.h"
#include "AviTabCore.h"
#include "Logger.h"
#include "platform/CrashHandler.h"
#include "AviTabBuildSettings.h"

std::shared_ptr<int> panelPowered;
std::shared_ptr<int> panelEnabled;
std::shared_ptr<float> panelBrightness;
std::shared_ptr<avitab::Settings> settings;
std::shared_ptr<XPlaneSimDriver> simDriver;
std::shared_ptr<XPlaneUiDriver> uiDriver;
std::shared_ptr<avitab::AviTabCore> aviTab;

PLUGIN_API int XPluginStart(char *outName, char *outSignature, char *outDescription) {
    strncpy(outName, "AviTab", 255);
    strncpy(outSignature, "org.solhost.folko.avitab", 255);

    XPLMDebugString("AviTab (" AVITAB_VERSION_STR ") - started\n");

    crash::registerHandler([] () { return XPLMGetMyID(); });

    try {
        XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
        curl_global_init(CURL_GLOBAL_ALL);

        simDriver = std::make_shared<XPlaneSimDriver>();

        auto fname(simDriver->getSettingsDir() / "avitab.prf");
        settings = std::make_shared<avitab::Settings>(fname);
        logger::info("Settings file: %s", fname.string().c_str());

        // REFACTOR - switch over to the Navitab logger and get it going right at the start
        logger::setStdOut(simDriver->getLogToStdout());
        logger::init(simDriver->getDataRootPath());
        logger::info("AviTab version " AVITAB_VERSION_STR);

        uiDriver = std::make_shared<XPlaneUiDriver>();

        aviTab = avitab::AviTabCore::CreateAviTabCore();

        simDriver->start(settings, aviTab, uiDriver);
        uiDriver->start(settings, aviTab, simDriver);
        aviTab->start(settings, simDriver, uiDriver);

        strncpy(outDescription, "A tablet to help in VR.", 255);
    } catch (const std::exception &e) {
        try {
            aviTab.reset();
            uiDriver.reset();
            simDriver.reset();
        } catch (...) {
            logger::error("SimDriverBase exception while destroying");
        }
        logger::error("Exception in XPluginStart: %s", e.what());
        strncpy(outDescription, e.what(), 255);
        return 0;
    }

    return 1;
}

PLUGIN_API int XPluginEnable(void) {
    XPLMDebugString("AviTab (" AVITAB_VERSION_STR ") - enabled\n");
    try {
        simDriver->enable();
        uiDriver->enable(simDriver->getAirplanePath());
        aviTab->enable();
    } catch (const std::exception &e) {
        logger::error("Exception in XPluginEnable: %s", e.what());
        return 0;
    }

    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID src, int msg, void *inParam) {
    if (msg == XPLM_MSG_PLANE_LOADED && inParam == 0) {
        try {
            simDriver->onPlaneLoad();
            uiDriver->onPlaneLoad(simDriver->getAirplanePath());
            aviTab->onPlaneLoad(uiDriver->isHeaderAppHidden());
        } catch (const std::exception &e) {
            logger::error("Exception in XPluginReceiveMessage: %s", e.what());
        }
    } else if ((msg == XPLM_MSG_ENTERED_VR) || (msg == XPLM_MSG_EXITING_VR)) {
        uiDriver->onVRswitch();
    }
}

PLUGIN_API void XPluginDisable(void) {
    try {
        aviTab->disable();
        uiDriver->disable();
        simDriver->disable();
    } catch (const std::exception &e) {
        logger::error("Exception in XPluginDisable: %s", e.what());
    }
    XPLMDebugString("AviTab (" AVITAB_VERSION_STR ") - disabled\n");
}

PLUGIN_API void XPluginStop(void) {
    logger::verbose("AviTab told to stop");
    try {
        aviTab->stop();
        uiDriver->stop();
        simDriver->stop();

        aviTab.reset();
        uiDriver.reset();
        simDriver.reset();

        curl_global_cleanup();
    } catch (const std::exception &e) {
        logger::error("Exception in XPluginStop: %s", e.what());
    }

    crash::unregisterHandler();
    logger::verbose("AviTab unloaded");

    XPLMDebugString("AviTab (" AVITAB_VERSION_STR ") - stopped\n");
}

#ifdef _WIN32
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    return TRUE;
}

#endif

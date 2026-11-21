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

#include "Settings.h"
#include "Logger.h"
#include "platform/Platform.h"
#include <nlohmann/json.hpp>
#include <iomanip>
#include <sstream>
#include <cassert>

using json = nlohmann::json;

namespace avitab {

static const int PREFS_VERSION = 3;

Settings::Settings(const std::filesystem::path & settingsFile)
:   filePath(settingsFile)
{
    database = std::make_shared<json>();
    listeners.push_back(std::make_pair("", nullptr));
    init();
    load();
    cleanUp();
    initMissing();
    
	//logger::verbose("Preferences json %s:", filePath.string().c_str());
	//logger::verbose("%s", database->dump(4).c_str());
	
    // Handle older json databases which have since been updated.
    // The version number is only changed if a setting has been moved, renamed, or removed.
    // Adding new settings generally does not require a version number increment.
    // If an upgrade over several versions (eg v1 to v4) is required, this done for each
    // later version in turn.

    auto loadedPrefsVersion = database->value(json::json_pointer("/general/prefs_version"), PREFS_VERSION);
    if (loadedPrefsVersion >= PREFS_VERSION) return;

    if (loadedPrefsVersion == 1) {
        upgrade1to2();
        ++loadedPrefsVersion;
    }
    if (loadedPrefsVersion == 2) {
        upgrade2to3();
        ++loadedPrefsVersion;
    }

    (*database)["general"]["prefs_version"] = PREFS_VERSION;
}

Settings::~Settings()
{
    try {
        saveAll();
    } catch (...) {
        // can't rescue the process at this point
    }
}

const nlohmann::json Settings::get(const std::string &key) {
    auto jp = nlohmann::json::json_pointer(key);
    nlohmann::json node = nlohmann::json::object();
    if ((*database).contains(jp)) {
        node = (*database)[jp];
    }
    return node;
}

void Settings::put(const std::string &key, nlohmann::json &value, int updater) {
    auto jp = nlohmann::json::json_pointer(key);
    (*database)[jp] = value;
    for (size_t i = 1; i < listeners.size(); ++i) {
        if (i == updater) continue;
        auto &l = listeners[i];
        if (l.second && l.first.size() && l.first.find(key) == 0) {
            l.second(updater);
        }
    }
}

int Settings::registerChangeListener(const std::string key, ListenerFunc fn) {
    auto id = listeners.size();
    assert(id);
    listeners.push_back(std::make_pair(key, fn));
    fn(0); // use the callback to do one first-time update
    return id;
}

void Settings::unregisterChangeListener(int id) {
    if ((id > 0) && (id < listeners.size())) {
        listeners[id] = std::make_pair("", nullptr);
    }
}

void Settings::init() {
    // full init not required, just defaults that aren't zero/false/empty
    *database = { { "general", { { "prefs_version", PREFS_VERSION },
                                 { "show_calibration_msg_on_load", true },
                                 { "show_overlays_in_airport_app", false },
                                 { "show_overlays_in_charts_app", false },
                                 { "show_fps", true } } },
                  { "overlay", { { "my_aircraft", true } } } };
}

void Settings::load() {
    json filedata;
    try {
        std::ifstream fin(filePath);
        fin >> filedata;
    } catch (const std::exception &e) {
        LOG_WARN("Could not load user settings from %s, using defaults", filePath.string().c_str());
    }
    for (const auto &i : filedata.items()) {
        (*database)[i.key()] = i.value();
    }
}

void Settings::upgrade1to2() {
    // version 1 preferences used the key 'pdfreading' which was changed to 'docreading' in version 2
    // to reflect the more general capabilities of the document reading apps.
    std::vector<std::string> apps {"chartsapp", "manualsapp"};
    for (auto& appname: apps) {
        if ((*database).contains(appname)) {
            if ((*database)[appname].contains("pdfreading")) {
                std::swap((*database)[appname]["pdfreading"], (*database)[appname]["docreading"]);
                (*database)[appname].erase("pdfreading");
            }
        }
    }
}

void Settings::upgrade2to3() {
    // version 3 added these preferences,
    // although the version update wasn't actually required for simple additions.
    (*database)["airports"]["sort"] = true;
    (*database)["airports"]["sort_ascending"] = true;
}

void Settings::cleanUp() {
    // Get rid of any other obsolete entries, sometimes these are transitory developer entries
    (*database)["general"].erase("remote_georefs_urls");
    (*database).erase("window");
}

void Settings::initMissing() {
    // Initialise "missing" settings, added to code since last prefs version update
    // But they don't need a new prefs version, just set to a default if they're not there

    if (!database->contains("/georefs/remote_urls"_json_pointer)) {
        // Use latest checked in version, but from Github Pages to allow higher rate limit
        const std::vector<std::string> default_url_list = { 
            "https://teamavitab.github.io/avitab_georefs/georefs/TeamAvitabGeorefs.zip" 
        };
        (*database)["georefs"]["remote_urls"] = default_url_list;
    }
}

void Settings::saveAll() {
    try {
        std::ofstream fout(filePath);
        fout << std::setw(4) << *database;
    } catch (const std::exception &e) {
        LOG_ERROR("Could not save user settings to %s", filePath.string().c_str());
    }
}

void ConfigBase::init(std::shared_ptr<Settings> db, const std::string &k, UpdateFunc fn) {
    settings = db;
    key = k;
    updCallback = fn;
    listenerId = settings->registerChangeListener(key, [this] (int id) { onUpdate(id); });
}

void ConfigBase::update() {
    pack();
    settings->put(key, json, listenerId);
}

void ConfigBase::onUpdate(int id) {
    if (id == 0) {
        // first time only, use this to populate the config struct
        json = settings->get(key);
        unpack();
    } else if (id != listenerId) {
        // some part of the config was updated by another module
        // get the updated json, but don't unpack it here, since
        // the callback might want to test which fields were changed.
        json = settings->get(key);
        updCallback();
    } else {
        // the config was updated by this listener, so do nothing
    }
}

void GeneralConfig::unpack() {
    version = json["prefs_version"];
    showFPS = json.value("show_fps", true);
    clockMode = json.value("clock_mode", 0);
    showAirportOverlays = json.value("show_overlays_in_airport_app", true);
    showChartOverlays = json.value("show_overlays_in_charts_app", true);
    showCalibrateMsgOnLoad = json.value("show_calibration_msg_on_load", true);
    userFixesFile = json.value("userfixes_file", "");
    nightMode = json.value("night_mode", false);
}

void GeneralConfig::pack() {
    json["show_fps"] = showFPS;
    json["clock_mode"] = clockMode;
    json["show_overlays_in_airport_app"] = showAirportOverlays;
    json["show_overlays_in_charts_app"]  = showChartOverlays;
    json["show_calibration_msg_on_load"] = showCalibrateMsgOnLoad;
    json["userfixes_file"]               = userFixesFile;
    json["night_mode"]                   = nightMode;
}


} /* namespace avitab */

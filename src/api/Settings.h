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

#include <nlohmann/json.hpp>
#include <filesystem>
#include <functional>
#include <list>
#include <vector>

namespace avitab {

class Settings {
public:
    Settings(const std::filesystem::path &settingsFile);
    ~Settings();

    // get a copy of a json object in the settings database (empty if non-existent)
    const nlohmann::json get(const std::string &key);
    // update (or add) a json object to the settings database and trigger listener callbacks
    void put(const std::string &key, nlohmann::json& value, int source = -1); // -1 is an unregisterd source

    using ListenerFunc = std::function<void(int id)>;
    int registerChangeListener(const std::string key, ListenerFunc fn);
    void unregisterChangeListener(int id);

    void saveAll();

private:
    std::vector<std::pair<std::string, ListenerFunc>> listeners;

private:
    void init();
    void upgrade1to2();
    void upgrade2to3();
    void cleanUp();
    void initMissing();
    void load();
    void save();

private:
    const std::filesystem::path filePath;
    std::shared_ptr<nlohmann::json> database;
};


class ConfigBase {
public:
    using UpdateFunc = std::function<void()>;
    void init(std::shared_ptr<Settings> db, const std::string &key, UpdateFunc fn);
    void update();
protected:
    virtual void unpack() = 0;
    virtual void pack() = 0;
private:
    void onUpdate(int id);
protected:
    std::shared_ptr<Settings> settings; // access to the settings object
    nlohmann::json json;                // representation of the configuration group as a json object, ie key-value pairs
    std::string key;                    // the key for this configuration group
    int listenerId;                     // the ID issued when registering as a listener for updates
    UpdateFunc updCallback;             // callback when config is updated elsewhere
};


struct GeneralConfig : public ConfigBase {
    int version;
    bool showFPS;               // TODO - move to header app group with next version update
    int clockMode;              // TODO - move to header app group with next version update
    bool showAirportOverlays;   // TODO - move to airport app group with next version update
    bool showChartOverlays;     // TODO - move to chart app group with next version update
    bool showCalibrateMsgOnLoad;// TODO - move to map app group with next version update
    std::string userFixesFile;  // TODO - move to navdb group with next version update
    bool nightMode;             // TODO - night mode needs a UI setting (header app?)

    void unpack() override;
    void pack() override;
};




} /* namespace avitab */

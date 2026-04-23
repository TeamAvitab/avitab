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
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include "platform/Platform.h"
#include "Config.h"

using json = nlohmann::json;

namespace avitab {

class missing_file : public std::exception { };

inline static std::shared_ptr<nlohmann::json> ReadConfig(const std::string& configFile) {
    // Raises an exception if file does not exist, or if there is a json format error when reading
    std::ifstream configStream(std::filesystem::u8path(configFile));
    if (!configStream) {
        throw missing_file();
    }
    auto cfg = std::make_shared<json>();
    configStream >> *cfg;
    return cfg;
}

Config::Config(const std::string& configFile) {
    // No recovery from any exceptions raised, just let them propogate.
    config = ReadConfig(configFile);
}

Config::Config(const std::string& configFile, const std::string &createDefault) {
    try {
        // First attempt to read a json config from the file.
        config = ReadConfig(configFile);
        return; // no exceptions - all done.
    } catch (const missing_file &e) {
        // Probably this is the first time that Avitab has been run after a clean install.
        // The installation packages no longer include files that might be updated by
        // the user, because we don't want subsequent installations to overwrite these.
        // Attempt to create the default file if missing and try again.
        std::ofstream configStream(std::filesystem::u8path(configFile));
        configStream << createDefault;
    }

    // Second attempt. If it fails just propagate the exception, no further recovery possible.
    config = ReadConfig(configFile);
}

std::string Config::getString(const std::string& pointer) {
    return (*config)[json::json_pointer(pointer)];
}

bool Config::getBool(const std::string& pointer) {
    return (*config)[json::json_pointer(pointer)];
}

int Config::getInt(const std::string& pointer) {
    return (*config)[json::json_pointer(pointer)];
}

} /* namespace avitab */

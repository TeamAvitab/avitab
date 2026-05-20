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
#include <chrono>
#include <thread>
#include <cassert>
#include <filesystem>
#include "NavLoaderXp.h"
#include "nav/NavDbManager.h"
#include "nav/InMemNavDb.h"
#include "nav/parsers/xplane/SceneryIndexParser.h"
#include "nav/loaders/xplane/AirportLoader.h"
#include "nav/loaders/xplane/FixLoader.h"
#include "nav/loaders/xplane/NavaidLoader.h"
#include "nav/loaders/xplane/AirwayLoader.h"
#include "nav/loaders/xplane/CIFPLoader.h"
#include "nav/models/Airport.h"
#include "Logger.h"

namespace xdata {

XplaneNavDataLoader::XplaneNavDataLoader(const navdb::NavDbManager & parent, const std::filesystem::path & xpir) :
    owner(parent),
    xpInstallRoot(xpir),
    xpVersion(0)
{
    if (std::filesystem::exists(xpInstallRoot/"Global Scenery"/"Global Airports"/"Earth nav data"/"apt.dat")) {
        xpVersion = 12;
    } else if (std::filesystem::exists(xpInstallRoot/"Resources"/"default scenery"/"default apt dat"/"Earth nav data"/"apt.dat")) {
        xpVersion = 11;
    } else {
        throw std::runtime_error("unknown X-Plane installation structure");
    }
    bool custom = false;
    if (std::filesystem::exists(xpInstallRoot/"Custom Data"/"earth_fix.dat") &&
            std::filesystem::exists(xpInstallRoot/"Custom Data"/"earth_nav.dat") &&
            std::filesystem::exists(xpInstallRoot/"Custom Data"/"earth_awy.dat") &&
            std::filesystem::exists(xpInstallRoot/"Custom Data"/"CIFP")) {
        navDataPath = xpInstallRoot/"Custom Data";
        custom = true;
    } else {
        navDataPath = xpInstallRoot/"Resources"/"default data";
    }
    logger::info("Loading %s NAV data from X-Plane v%d installation @ %s", custom?"custom":"default", xpVersion, xpInstallRoot.string().c_str());
}

void XplaneNavDataLoader::discoverSceneries() {
    logger::verbose("Discovering user sceneries...");
    try {
        SceneryIndexParser parser(xpInstallRoot/"Custom Scenery"/"scenery_packs.ini");
        parser.setAcceptor([this](const std::filesystem::path &entry) {
            std::filesystem::path aptFilePath;
            if (entry.is_absolute()) {
                aptFilePath = entry/"Earth nav data"/"apt.dat";
            } else {
                aptFilePath = xpInstallRoot/entry/"Earth nav data"/"apt.dat";
            }
            if (std::filesystem::exists(aptFilePath)) {
                customSceneries.push_back(aptFilePath);
            }
        });
        parser.loadCustomScenery();
    } catch (const std::exception &e) {
        logger::warn("Could not load scenery_packs.ini: %s", e.what());
    }
}

void XplaneNavDataLoader::load() {
    auto startAt = std::chrono::steady_clock::now();
    xpnavdb = std::make_shared<navdb::InMemoryNavDb>();
    loadCustomScenery();
    loadAirports();
    loadFixes();
    loadNavaids();
    loadAirways();
    loadProcedures();
    // ISSUE - load user additions, see https://github.com/TeamAvitab/avitab/issues/58
    logger::verbose("Building NAV data indices...");
    xpnavdb->buildGeoIndex();

    auto duration = std::chrono::steady_clock::now() - startAt;
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    logger::info("Loaded NAV data in %.2f seconds", millis / 1000.0f);
}

bool XplaneNavDataLoader::shouldCancelLoading() {
    return owner.stopRequested();
}

void XplaneNavDataLoader::loadCustomScenery() {
    const AirportLoader loader(this);

    for (auto &aptDatPath: customSceneries) {
        try {
            logger::info("Loading custom scenery airport from %s", aptDatPath.string().c_str());
            loader.load(aptDatPath);
        } catch (const std::exception &e) {
            logger::warn("Unable to parse custom scenery: %s", e.what());
        }
    }
}

void XplaneNavDataLoader::loadAirports() {
    const AirportLoader loader(this);

    std::filesystem::path aptPath;
    if (xpVersion == 11) {
        aptPath = xpInstallRoot/"Resources"/"default scenery"/"default apt dat"/"Earth nav data"/"apt.dat";
    } else if (xpVersion == 12) {
        aptPath = xpInstallRoot/"Global Scenery/Global Airports/Earth nav data/apt.dat";
    }

    logger::verbose("Loading default airports from %s", aptPath.string().c_str());

    try {
        loader.load(aptPath);
    } catch (...) {
        logger::error("Error during parsing, default airports did not load");
    }
}

void XplaneNavDataLoader::loadFixes() {
    FixLoader loader(this);
    auto fixFile = navDataPath/"earth_fix.dat";
    logger::info("Loading fixes from %s ...", fixFile.string().c_str());
    try {
        loader.load(fixFile);
    } catch (...) {
        logger::error("Error during parsing, fixes were not loaded");
    }
}

void XplaneNavDataLoader::loadNavaids() {
    NavaidLoader loader(this);
    auto navFile = navDataPath/"earth_nav.dat";
    logger::info("Loading navaids from %s ...", navFile.string().c_str());
    try {
        loader.load(navFile);
    } catch (...) {
        logger::error("Error during parsing, navaids were not loaded");
    }
}

void XplaneNavDataLoader::loadAirways() {
    AirwayLoader loader(this);
    auto awyFile = navDataPath/"earth_awy.dat";
    logger::info("Loading airways from %s ...", awyFile.string().c_str());
    try {
        loader.load(awyFile);
    } catch (...) {
        logger::error("Error during parsing, airways were not loaded");
    }
}

void XplaneNavDataLoader::loadProcedures() {
    CIFPLoader loader(this);
    auto cifpDir = navDataPath/"CIFP";
    logger::info("Loading procedures from %s/* ...", cifpDir.string().c_str());
    xpnavdb->forEachAirport([this, &loader, cifpDir] (std::shared_ptr<navdb::Airport> ap) {
        try {
            auto apm = std::dynamic_pointer_cast<navdb::AirportModel>(ap);
            loader.load(apm, cifpDir/(ap->getIdent() + ".dat"));
        } catch (...) {
            // many airports do not have CIFP data, so ignore silently
        }
        if (shouldCancelLoading()) {
            throw std::runtime_error("Cancelled");
        }
    });
}

} /* namespace xdata */

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
#include "FixLoader.h"
#include "NavLoaderXp.h"
#include "nav/InMemNavDb.h"
#include "nav/parsers/xplane/FixParser.h"
#include "nav/models/NavFix.h"
#include "nav/models/Airport.h"
#include "nav/models/Region.h"
#include "Logger.h"

namespace xdata {

FixLoader::FixLoader(XplaneNavDataLoader* mgr):
    loadMgr(mgr),
    world(mgr->getDatabase())
{
}

void FixLoader::load(const std::filesystem::path& file) {
    FixParser parser(file);
    parser.setAcceptor([this] (const FixData &data) {
        try {
            onFixLoaded(data);
        } catch (const std::exception &e) {
            logger::warn("Can't parse fix %s: %s", data.id.c_str(), e.what());
        }
        if (loadMgr->shouldCancelLoading()) {
            throw std::runtime_error("Cancelled");
        }
    });
    parser.loadFixes();
}

void FixLoader::onFixLoaded(const FixData& fix) {
    if (fix.terminalAreaId == "ENRT") {
        loadEnrouteFix(fix);
    } else {
        loadTerminalFix(fix);
    }
}

void FixLoader::loadEnrouteFix(const FixData& fix) {
    auto region = world->findOrCreateRegion(fix.icaoRegion);
    auto loc = world::Location::fromGCS(fix.latitude, fix.longitude);

    auto fixModel = std::make_shared<navdb::NavFixModel>(region, fix.id, loc);
    world->addFix(fixModel);
}

void FixLoader::loadTerminalFix(const FixData& fix) {
    auto region = world->findOrCreateRegion(fix.icaoRegion);
    auto loc = world::Location::fromGCS(fix.latitude, fix.longitude);

    auto fixModel = std::make_shared<navdb::NavFixModel>(region, fix.id, loc);

    auto airport = std::dynamic_pointer_cast<navdb::AirportModel>(world->findAirportByID(fix.terminalAreaId));
    if (airport) {
        airport->addTerminalFix(fixModel);
    }
}

} /* namespace xdata */

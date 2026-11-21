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
#include "NavaidLoader.h"
#include "NavLoaderXp.h"
#include "nav/InMemNavDb.h"
#include "nav/parsers/xplane/NavaidParser.h"
#include "nav/models/NavFix.h"
#include "nav/models/Airport.h"
#include "nav/models/Runway.h"
#include "nav/models/Region.h"
#include "Logger.h"

namespace xdata {

NavaidLoader::NavaidLoader(XplaneNavDataLoader* mgr):
    loadMgr(mgr),
    world(mgr->getDatabase())
{
}

void NavaidLoader::load(const std::filesystem::path& file) {
    NavaidParser parser(file);
    parser.setAcceptor([this] (const NavaidData &data) {
        try {
            onNavaidLoaded(data);
        } catch (const std::exception &e) {
            logger::warn("Can't parse navaid %s: %s", data.id.c_str(), e.what());
        }
        if (loadMgr->shouldCancelLoading()) {
            throw std::runtime_error("Cancelled");
        }
    });
    parser.loadNavaids();
    addPendingGLSs();
    logger::info("Loaded %d FPAP and %d GLS approaches, skipped %d FPAP/GLS approaches of airports or runways not in the NAV database",
                 loadedFPAPs, loadedGLSs, skippedFPAPs);
}

void NavaidLoader::onNavaidLoaded(const NavaidData& navaid) {
    if ((navaid.type == NavaidData::Type::FPAP) || (navaid.type == NavaidData::Type::LTP_FTP) || (navaid.type == NavaidData::Type::GLS)) {
        // skip approaches to airports that are not in the NAV database
        if (!world->findAirportByID(navaid.terminalRegion)) {
            if (navaid.type != NavaidData::Type::LTP_FTP) {
                ++skippedFPAPs;
            }
            return;
        }
    }
    if (navaid.type == NavaidData::Type::GLS) {
        // a GLS may also be described by an FPAP (type 14), which can come later in the file
        pendingGLSs.push_back(navaid);
        return;
    }
    if (navaid.type == NavaidData::Type::LTP_FTP) {
        // not a fix of its own, only the start of an FPAP's final approach course
        onThresholdLoaded(navaid);
        return;
    }

    bool new_dme = (navaid.type == NavaidData::Type::DME_SINGLE || navaid.type == NavaidData::Type::DME_COMP);
    bool new_vor = (navaid.type == NavaidData::Type::VOR);
    bool new_ndb = (navaid.type == NavaidData::Type::NDB);
    bool new_fpap = (navaid.type == NavaidData::Type::FPAP);
    auto fix = std::dynamic_pointer_cast<navdb::NavFixModel>(world->findFixByRegionAndID(navaid.icaoRegion, navaid.id));
    auto airport = std::dynamic_pointer_cast<navdb::AirportModel>(world->findAirportByID(navaid.terminalRegion));
    bool dontPair = false;
    if (fix) {
        bool old_ndb = (bool)fix->getNDB();
        bool old_vor_dme = fix->getVOR() || fix->getDME();
        dontPair = ((new_vor || new_dme) && old_ndb) || (new_ndb && old_vor_dme);
        // FPAP procedure identifiers (e.g. R07CY) repeat at different airports of a region
        dontPair |= new_fpap;
    }
    if (!fix || dontPair) {
        auto loc = world::Location::fromGCS(navaid.latitude, navaid.longitude);
        auto region = world->findOrCreateRegion(navaid.icaoRegion);
        fix = std::make_shared<navdb::NavFixModel>(region, navaid.id, loc);
        world->addFix(fix);
    }

    if ((navaid.type == NavaidData::Type::ILS_LOC) || (navaid.type == NavaidData::Type::LOC)) {
        std::istringstream rwyAndDesc(navaid.name);
        std::string rwy;
        std::string desc;
        rwyAndDesc >> rwy;
        rwyAndDesc >> desc;
        navdb::RadioFrequency ilsFrq = navdb::RadioFrequency(navaid.radio, 2, navdb::FrequencyUnit::MHZ, desc);
        auto ils = std::make_unique<navdb::ILSLocalizerModel>(ilsFrq, navaid.range, navaid.bearing, navaid.bearingMagnetic, navaid.type == NavaidData::Type::LOC);
        fix->attachILSLocalizer(ils);

        if (airport) {
            airport->attachILSData(rwy, fix);
        }
    } else if (new_fpap) {
        std::istringstream rwyAndType(navaid.name);
        std::string rwy;
        std::string type;
        rwyAndType >> rwy;
        rwyAndType >> type;
        auto fpap = std::make_unique<navdb::FPAPModel>(navaid.radio, navaid.bearing, rwy, type);
        ApproachKey key(navaid.terminalRegion, navaid.id);
        auto pending = pendingThresholds.find(key);
        if (pending != pendingThresholds.end()) {
            fpap->setThreshold(pending->second.location);
            fpap->setGlidepathAngle(pending->second.glidepathAngle);
            pendingThresholds.erase(pending);
        }
        fpaps[key] = fpap.get();
        ++loadedFPAPs;
        fix->attachFPAP(fpap);

        if (airport) {
            airport->attachFPAPData(rwy, fix);
        }
    } else if (new_ndb) {
        navdb::RadioFrequency ndbFrq = navdb::RadioFrequency(navaid.radio, 0, navdb::FrequencyUnit::KHZ, navaid.name);
        auto ndb = std::make_unique<navdb::NDBModel>(ndbFrq, navaid.range);
        fix->attachNDB(ndb);
    } else if (new_vor) {
        navdb::RadioFrequency vorFrq = navdb::RadioFrequency(navaid.radio, 2, navdb::FrequencyUnit::MHZ, navaid.name);
        auto vor = std::make_unique<navdb::VORModel>(vorFrq, navaid.range, navaid.bearing);
        fix->attachVOR(vor);
    } else if (new_dme) {
        navdb::RadioFrequency dmeFreq = navdb::RadioFrequency(navaid.radio, 2, navdb::FrequencyUnit::MHZ, navaid.name);
        auto dme = std::make_unique<navdb::DMEModel>(dmeFreq, navaid.range, navaid.type == NavaidData::Type::DME_COMP);
        fix->attachDME(dme);
    }
}

void NavaidLoader::addPendingGLSs() {
    for (auto &gls: pendingGLSs) {
        // prefer the FPAP of the same approach (XP-NAV1150/1200 allow "GLS" as FPAP approach type)
        if (fpaps.find(ApproachKey(gls.terminalRegion, gls.id)) == fpaps.end()) {
            onGLSLoaded(gls);
        }
    }
    pendingGLSs.clear();
}

void NavaidLoader::onGLSLoaded(const NavaidData& navaid) {
    // In X-Plane's data a GLS usually has no FPAP and no LTP/FTP, only its GBAS ground station
    // (type 15), which gives course and runway. Draw it like an FPAP, starting at that runway end.
    std::istringstream rwyAndType(navaid.name);
    std::string rwy;
    std::string type;
    rwyAndType >> rwy;
    rwyAndType >> type;

    auto airport = std::dynamic_pointer_cast<navdb::AirportModel>(world->findAirportByID(navaid.terminalRegion));
    std::shared_ptr<navdb::RunwayModel> runway;
    try {
        runway = airport ? airport->getClosestRunway(rwy) : nullptr;
    } catch (const std::exception &) {
        // malformed runway name
    }
    if (!runway) {
        ++skippedFPAPs;
        return;
    }

    // procedure identifiers (e.g. G08A) repeat at different airports, so always a new fix
    auto loc = world::Location::fromGCS(navaid.latitude, navaid.longitude);
    auto region = world->findOrCreateRegion(navaid.icaoRegion);
    auto fix = std::make_shared<navdb::NavFixModel>(region, navaid.id, loc);
    world->addFix(fix);

    auto gls = std::make_unique<navdb::FPAPModel>(navaid.radio, navaid.bearing, rwy, type);
    gls->setThreshold(runway->getLocation());
    gls->setGlidepathAngle(navaid.glidepathAngle);
    ++loadedGLSs;
    fix->attachFPAP(gls);
    runway->addNavaidFPAP(fix);
}

void NavaidLoader::onThresholdLoaded(const NavaidData& navaid) {
    auto loc = world::Location::fromGCS(navaid.latitude, navaid.longitude);
    ApproachKey key(navaid.terminalRegion, navaid.id);
    auto it = fpaps.find(key);
    if (it != fpaps.end()) {
        it->second->setThreshold(loc);
        it->second->setGlidepathAngle(navaid.glidepathAngle);
    } else {
        pendingThresholds[key] = Threshold{loc, navaid.glidepathAngle};
    }
}

} /* namespace xdata */

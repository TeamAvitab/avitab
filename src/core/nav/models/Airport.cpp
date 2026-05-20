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
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <cassert>
#include "Airport.h"
#include "Runway.h"
#include "Heliport.h"
#include "NavFix.h"
#include "Procedures.h"
#include "Logger.h"

namespace navdb {

void AirportModel::setICAOCode(const std::string& code) {
    icaoCode = code;
    displayID = code;
}

void AirportModel::setFAACode(const std::string& code) {
    faaCode = code;
    displayID = code;
}

void AirportModel::setLocalCode(const std::string& code) {
    localCode = code;
    displayID = code;
}

void AirportModel::setLocation(const world::Location& loc) {
    location = loc;
    if (!locSW.isValid()) locSW = loc;
    if (!locNE.isValid()) locNE = loc;
}

void AirportModel::addRunway(std::shared_ptr<RunwayModel> rwy) {
    runways.insert(std::make_pair(rwy->getIdent(), rwy));
    auto &loc = rwy->getLocation();
    locSW = locSW.areaSouthWest(loc);
    locNE = locNE.areaNorthEast(loc);
}

void AirportModel::addRunwayEnds(std::shared_ptr<RunwayModel> rwy1, std::shared_ptr<RunwayModel> rwy2) {
    runwayPairs.insert(std::make_pair(rwy1, rwy2));
}

void AirportModel::addHeliport(std::shared_ptr<HeliportModel> port) {
    heliports.insert(std::make_pair(port->getIdent(), port));
}

void AirportModel::addTerminalFix(std::shared_ptr<NavFixModel> fix) {
    terminalFixes.insert(std::make_pair(fix->getIdent(), fix));
}

std::shared_ptr<NavFix> navdb::AirportModel::getTerminalFix(const std::string& id) {
    auto it = terminalFixes.find(id);
    if (it == terminalFixes.end()) {
        return nullptr;
    }
    return it->second;
}

void AirportModel::attachILSData(const std::string& rwyName, std::shared_ptr<NavFix> fix) {
    auto rwy = getClosestRunway(rwyName);
    if (!rwy) {
        throw std::runtime_error("Unknown runway " + rwyName + " for airport " + ident);
    }
    rwy->setNavaidILS(fix);
}

std::shared_ptr<RunwayModel> AirportModel::getClosestRunway(const std::string& name) {
    auto rwy = std::dynamic_pointer_cast<RunwayModel>(getRunwayByName(name));
    if (rwy) return rwy;

    // if there's no exact match then find the 'intended' runway
    // the nav data is often newer than apt.dat, so we must sometimes rename runways

    int wantHeading = std::stoi(name.substr(0, 2)) * 10;

    if (name.empty()) {
        return nullptr;
    }

    for (auto it = runways.begin(); it != runways.end(); ++it) {
        if (it->first.empty()) {
            continue;
        }

        if (std::isalpha(name.back()) || std::isalpha(it->first.back())) {
            if (name.back() != it->first.back()) {
                // make sure we don't rename 16L to 17C etc.
                continue;
            }
        }

        int curHeading = std::stoi(it->first.substr(0, 2)) * 10;
        int diff = wantHeading - curHeading;
        if (diff < -180) {
            diff += 360;
        }
        if (diff > 180) {
            diff -= 360;
        }
        if (std::abs(diff) <= 30) {
            //logger::verbose("Renaming runway %s to %s for airport %s due to newer nav data", it->first.c_str(), name.c_str(), ident.c_str());
            rwy = it->second;
            runways.erase(it);
            rwy->rename(name);
            runways.insert(std::make_pair(rwy->getIdent(), rwy));
            return rwy;
        }
    }
    return nullptr;
}

const std::shared_ptr<Runway> AirportModel::getRunwayByName(const std::string& rw) const {
    auto rwy = runways.find(rw);
    if (rwy == runways.end()) {
        return nullptr;
    }
    return rwy->second;
}

const std::shared_ptr<Runway> AirportModel::getOppositeRunwayEnd(const std::shared_ptr<Runway> rw) const {
    std::string rwyName = rw->getIdent();
    for (auto &rwys: runwayPairs) {
        if (rwys.first->getIdent() == rwyName) {
            return rwys.second;
        }
        if (rwys.second->getIdent() == rwyName) {
            return rwys.first;
        }
    };
    return nullptr;
}

void AirportModel::forEachRunway(std::function<void(const std::shared_ptr<Runway>)> f) const {
    for (auto &rwy: runways) {
        f(rwy.second);
    }
}

void AirportModel::forEachRunwayPair(std::function<void(const std::shared_ptr<Runway>, const std::shared_ptr<Runway>)> f) const {
    for (auto &rwys: runwayPairs) {
        f(rwys.first, rwys.second);
    }
}

float AirportModel::getLongestRunwayLength() const {
    float longestRunwayLength = 0;
    for (auto &rwy: runways) {
        longestRunwayLength = std::fmax(rwy.second->getLength(), longestRunwayLength); // Ignore NaN
    }
    return longestRunwayLength;
}

void AirportModel::forEachHeliport(std::function<void(const std::shared_ptr<Heliport>)> f) const {
    for (auto &port: heliports) {
        f(port.second);
    }
}

bool AirportModel::hasHeliports() const {
    return !heliports.empty();
}

bool AirportModel::hasOnlyHeliports() const {
    return runways.empty() && !heliports.empty();
}

bool AirportModel::hasOnlyWaterRunways() const {
    bool foundWater = false;
    for (const auto &rwy: runways) {
        if (rwy.second->isWater()) {
            foundWater = true;
        } else {
            return false;
        }
    }
    return foundWater;
}

bool AirportModel::hasControlTower() const {
    return (atcFrequencies.count(ATCserviceType::TWR) >= 1);
}

bool AirportModel::hasATCFrequencies() const {
    return (atcFrequencies.size() >= 1);
}

bool AirportModel::hasHardRunway() const {
    for (const auto &rwy: runways) {
        if (rwy.second->hasHardSurface()) {
            return true;
        }
    }
    return false;
}

void AirportModel::addSID(std::shared_ptr<SIDModel> sid) {
    sids.insert(std::make_pair(sid->getIdent(), sid));
}

void AirportModel::addSTAR(std::shared_ptr<STARModel> star) {
    stars.insert(std::make_pair(star->getIdent(), star));
}

void AirportModel::addApproach(std::shared_ptr<ApproachModel> approach) {
    approaches.insert(std::make_pair(approach->getIdent(), approach));
}

std::vector<std::shared_ptr<SID>> AirportModel::getSIDs() const {
    std::vector<std::shared_ptr<SID>> res;
    for (auto &it: sids) {
        res.push_back(it.second);
    }
    return res;
}

std::shared_ptr<SID> AirportModel::getSIDByName(std::string sidName) const {
    if (sidName.empty()) {
        return nullptr;
    }
    auto sid = sids.find(sidName);
    if (sid == sids.end()) {
        std::stringstream ss;
        for (auto &it: sids) {
            ss << it.first << " ";
        }
        logger::warn("Couldn't find SID '%s', %s SIDs are %s",
            sidName.c_str(), getIdent().c_str(), ss.str().c_str());
        return nullptr;
    }
    return sid->second;
}

std::vector<std::shared_ptr<STAR>> AirportModel::getSTARs() const {
    std::vector<std::shared_ptr<STAR>> res;
    for (auto &it: stars) {
        res.push_back(it.second);
    }
    return res;
}

std::shared_ptr<STAR> AirportModel::getSTARByName(std::string starName) const {
    if (starName.empty()) {
        return nullptr;
    }
    auto star = stars.find(starName);
    if (star == stars.end()) {
        std::stringstream ss;
        for (auto &it: stars) {
            ss << it.first << " ";
        }
        logger::warn("Couldn't find STAR '%s', %s STARs are %s",
            starName.c_str(), getIdent().c_str(), ss.str().c_str());
        return nullptr;
    }
    return star->second;
}

std::vector<std::shared_ptr<Approach>> AirportModel::getApproaches() const {
    std::vector<std::shared_ptr<Approach>> res;
    for (auto &it: approaches) {
        res.push_back(it.second);
    }
    return res;
}

std::shared_ptr<Approach> AirportModel::getApproachByName(std::string appName) const {
    if (appName.empty()) {
        return nullptr;
    }
    auto approach = approaches.find(appName);
    if (approach == approaches.end()) {
        std::stringstream ss;
        for (auto &it: approaches) {
            ss << it.first << " ";
        }
        logger::warn("Couldn't find APP '%s', %s APPs are %s",
            appName.c_str(), getIdent().c_str(), ss.str().c_str());
        return nullptr;
    }
    return approach->second;
}

const world::Location& AirportModel::getLocation() const {
    assert(location.isValid());
    return location;
}

const world::Location& AirportModel::getCornerSW() const {
    return (locSW.isValid()) ? locSW : location;
}

const world::Location& AirportModel::getCornerNE() const {
    return (locNE.isValid()) ? locNE : location;
}

std::string AirportModel::getInitialATCContactInfo() const {
    static const ATCserviceType prioritisedATCType[] {
        ATCserviceType::RECORDED,
        ATCserviceType::TWR,
        ATCserviceType::UNICOM,
        ATCserviceType::APP,
        ATCserviceType::DEP,
        ATCserviceType::CLD,
        ATCserviceType::GND
    };
    std::string initialATCContact = "";
    for (auto atcType: prioritisedATCType) {
        if (atcFrequencies.count(atcType) > 0 && !atcFrequencies.at(atcType).empty()) {
            std::string desc;
            switch(atcType) {
                case(navdb::ATCserviceType::RECORDED): desc = "ATIS"; break;
                case(navdb::ATCserviceType::TWR):      desc = "TWR";  break;
                case(navdb::ATCserviceType::UNICOM):   desc = "UCOM"; break;
                case(navdb::ATCserviceType::APP):      desc = "APP";  break;
                case(navdb::ATCserviceType::DEP):      desc = "DEP";  break;
                case(navdb::ATCserviceType::CLD):      desc = "CLD";  break;
                case(navdb::ATCserviceType::GND):      desc = "GND";  break;
                default: desc = ""; break;
            }
            initialATCContact = desc + "-" + atcFrequencies.at(atcType).at(0).getFrequencyString(false);
            break;
        }
    }
    return initialATCContact;
}

} /* namespace navdb */

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
#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <memory>
#include <cassert>
#include "InMemNavDb.h"
#include "nav/models/Airport.h"
#include "nav/models/Airway.h"
#include "nav/models/NavFix.h"
#include "nav/models/Region.h"
#include "nav/models/UserFix.h"
#include "WorldGeometry.h"
#include "Logger.h"
#include "platform/Platform.h"

namespace navdb {

InMemoryNavDb::InMemoryNavDb()
{
}

std::shared_ptr<Airport> InMemoryNavDb::findAirportByID(const std::string& id) const {
    std::string cleanId = platform::upper(id);
    cleanId.erase(std::remove(cleanId.begin(), cleanId.end(), ' '), cleanId.end());

    auto it = airports.find(cleanId);
    if (it == airports.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}

std::shared_ptr<Airport> InMemoryNavDb::findAirportByCode(const std::string& id) const {
    std::string cleanId = platform::upper(id);
    cleanId.erase(std::remove(cleanId.begin(), cleanId.end(), ' '), cleanId.end());
    std::vector<std::string> codes;

    for (auto &it: airports) {
        codes = {it.second->getICAOCode(), it.second->getFAACode(), it.second->getLocalCode()};
        if (std::find(codes.begin(), codes.end(), cleanId) != codes.end()) {
            return it.second;
        }
    }
    return findAirportByID(id);
}

std::vector<std::shared_ptr<Airport>> InMemoryNavDb::findAirport(const std::string& keyWord) const {
    std::vector<std::shared_ptr<Airport>> res;

    std::string key = platform::lower(keyWord);

    auto directfind = findAirportByID(key);
    if (directfind) {
        res.push_back(directfind);
    }

    for (auto &it: airports) {
        std::ostringstream codes;
        codes << it.second->getIdent() << " " << it.second->getICAOCode() << " " << it.second->getFAACode() << " " << it.second->getLocalCode() << " " << it.second->getName();
        if (platform::lower(codes.str()).find(key) != std::string::npos) {
            if (std::find(res.begin(), res.end(), it.second) != res.end()) {
                continue;
            }
            res.push_back(it.second);
            if (res.size() >= MAX_SEARCH_RESULTS) {
                break;
            }
        }
    }

    return res;
}

std::shared_ptr<NavFix> InMemoryNavDb::findFixByRegionAndID(const std::string& region, const std::string& id) const {
    // this is only used by map calibration, user fixes are not returned
    auto range = navFixes.equal_range(id);

    for (auto it = range.first; it != range.second; ++it) {
        auto r = it->second->getRegion();
        if (r && (r->getIdent() == region)) {
            return it->second;
        }
    }

    return nullptr;
}

std::vector<std::shared_ptr<NavFix>> InMemoryNavDb::findFix(const std::string &id) const {
    std::vector<std::shared_ptr<NavFix>> fixes;

    auto range = navFixes.equal_range(id);
    for (auto it = range.first; it != range.second; ++it) {
        fixes.push_back(it->second);
    }

    return fixes;
}

void InMemoryNavDb::forEachAirport(std::function<void(std::shared_ptr<Airport>)> f) {
    for (auto it: airports) {
        f(it.second);
    }
}

std::shared_ptr<RegionModel> InMemoryNavDb::findOrCreateRegion(const std::string& id) {
    auto iter = regions.find(id);
    if (iter == regions.end()) {
        auto ptr = std::make_shared<RegionModel>(id);
        regions.insert(std::make_pair(id, ptr));
        return ptr;
    }
    return iter->second;
}

std::shared_ptr<AirportModel> InMemoryNavDb::findOrCreateAirport(const std::string& id) {
    auto iter = airports.find(id);
    if (iter == airports.end()) {
        auto ptr = std::make_shared<AirportModel>(id);
        airports.insert(std::make_pair(id, ptr));
        return ptr;
    }
    return iter->second;
}

std::shared_ptr<AirwayModel> InMemoryNavDb::findOrCreateAirway(const std::string& name, AirwayLevel lvl) {
    auto range = airways.equal_range(name);

    for (auto it = range.first; it != range.second; ++it) {
        if (it->second->getLevel() == lvl) {
            return it->second;
        }
    }

    // not found -> insert
    auto awy = std::make_shared<AirwayModel>(name, lvl);
    airways.insert(std::make_pair(name, awy));
    return awy;
}

std::vector<NavDatabase::Connection> &InMemoryNavDb::getConnections(std::shared_ptr<Node> from) {
    auto iter = connections.find(from);
    if (iter != connections.end()) {
        return iter->second;
    }
    return noConnection;
}

bool InMemoryNavDb::areConnected(std::shared_ptr<Node> from, const std::shared_ptr<Node> to) {
    for (auto &c: getConnections(from)) {
        if (c.second == to) {
            return true;
        }
    }
    return false;
}

void InMemoryNavDb::addRegion(const std::string &code) {
    findOrCreateRegion(code);
}

std::shared_ptr<Region> InMemoryNavDb::getRegion(const std::string &id) {
    auto iter = regions.find(id);
    if (iter == regions.end()) {
        return nullptr;
    }
    return iter->second;
}

void InMemoryNavDb::connectTo(std::shared_ptr<Node> from, std::shared_ptr<NodeLink> via, std::shared_ptr<Node> to) {
    auto iter = connections.find(from);
    if (iter == connections.end()) {
        connections[from] = std::vector<NavDatabase::Connection>();
        iter = connections.find(from);
    }
    (iter->second).push_back(std::make_pair(via, to));
}

void InMemoryNavDb::addFix(std::shared_ptr<NavFixModel> fix)
{
    fix->setGlobal();
    navFixes.insert(std::make_pair(fix->getIdent(), fix));
    // fixes may be added after the initial loading of the NAV world.
    // if so, register the node independently here
    if (indexingComplete) {
        addToGeoIndex(fix);
    }
}

void InMemoryNavDb::buildGeoIndex() {
    if (indexingComplete) return;
    for (auto it: airports) {
        addToGeoIndex(it.second);
    }
    for (auto it: navFixes) {
        addToGeoIndex(it.second);
    }
    indexingComplete = true;
}

void InMemoryNavDb::addToGeoIndex(std::shared_ptr<Node> n) {
    auto &loc = n->getLocation();
    int lat = (int) loc.latDegrees();
    int lon = (int) loc.lonDegrees();
    allNodes[std::make_pair(lat, lon)].push_back(n);
}

int InMemoryNavDb::maxDensity(const world::Location &bottomLeft, const world::Location &topRight) {
    int m = 0;

    // nodes are grouped by integer lat/lon 'squares'.
    int latl = std::max((int)std::floor(bottomLeft.latDegrees()), -90);
    int lath = std::min((int)std::ceil(topRight.latDegrees()), 89);
    int lonl = (int)std::floor(bottomLeft.lonDegrees());
    int lonh = (int)std::ceil(topRight.lonDegrees());

    // the area might span the -180/180 meridian. bias it here, normalise again in the iteration
    if (lonh < lonl) { lonh += 360; }
    for (int laty = latl; laty <= lath; ++laty) {
        for (int lonx = lonl; lonx <= lonh; ++lonx) {
            int normx = (lonx >= 180) ? (lonx - 360) : lonx;
            auto it = allNodes.find(std::make_pair(laty, normx));
            if (it == allNodes.end()) continue;
            m = std::max(m, (int)it->second.size());
        }
    }

    // pretend that each grid area has 'max' nodes in it, and report the total number of visible nodes that
    // would be seen if this was the case.
    float mapArea = (topRight.lonDegrees() > bottomLeft.lonDegrees())
                    ? (topRight.latDegrees() - bottomLeft.latDegrees()) * (topRight.lonDegrees() - bottomLeft.lonDegrees())
                    : (topRight.latDegrees() - bottomLeft.latDegrees()) * (360 + topRight.lonDegrees() - bottomLeft.lonDegrees());
    return (int)(mapArea * m);
}

void InMemoryNavDb::visitNodes(const world::Location& bottomLeft, const world::Location &topRight, NodeAcceptor callback, int filter) {
    // nodes are grouped by integer lat/lon 'squares'.
    int latl = std::max((int)std::floor(bottomLeft.latDegrees()), -90);
    int lath = std::min((int)std::ceil(topRight.latDegrees()), 89);
    int lonl = (int)std::floor(bottomLeft.lonDegrees());
    int lonh = (int)std::ceil(topRight.lonDegrees());

    // the area might span the -180/180 meridian. bias it here, normalise again in iteration
    if (lonh < lonl) { lonh += 360; }

    for (int laty = latl; laty <= lath; ++laty) {
        for (int lonx = lonl; lonx <= lonh; ++lonx) {
            int normx = (lonx >= 180) ? (lonx - 360) : lonx;
            std::pair<int, int> key = std::make_pair(laty, normx);
            auto it = allNodes.find(key);
            if (it == allNodes.end()) {
                continue;
            }
            for (auto node: it->second) {
                if (!node->getLocation().isContainedWithin(bottomLeft, topRight)) continue;
                bool accept = false;
                if (node->isAirport()) {
                    if (std::dynamic_pointer_cast<Airport>(node)->hasControlTower()) {
                        accept = (filter & NavDatabase::VISIT_TOWERED_AIRPORTS);
                    } else {
                        accept = (filter & NavDatabase::VISIT_OTHER_AIRPORTS);
                    }
                } else if (node->isFix()) {
                    auto f = std::dynamic_pointer_cast<Fix>(node);
                    if (f->hasNavaid()) {
                        accept = (filter & NavDatabase::VISIT_NAVAIDS);
                    } else if (f->isUserFix()) {
                        accept = (filter & NavDatabase::VISIT_USER_FIXES);
                    } else {
                        accept = (filter & NavDatabase::VISIT_FIXES);
                    }
                }
                if (accept) callback(node.get());
            }
        }
    }
}

void InMemoryNavDb::clearUserFixes()
{
    // existing user fixes are not currently removed - this may be considered a feature!
}

void InMemoryNavDb::addUserFix(std::shared_ptr<UserFix> uf)
{
    auto ufm = std::dynamic_pointer_cast<UserFixModel>(uf);
    assert(ufm);
    auto i = userFixes.find(ufm->getIdent());
    if (i != userFixes.end()) {
        // existing user fix with same ID. reuse, but update with latest info
        i->second->update(uf->getType(), uf->getName(), uf->getLocation());
    } else {
        userFixes[ufm->getIdent()] = ufm;
        addToGeoIndex(ufm);
    }
}

} /* namespace navdb */

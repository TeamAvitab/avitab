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

#include <map>
#include <string>
#include <vector>
#include <atomic>
#include "NavDbManager.h"
#include "WorldGeometry.h"

namespace navdb {

class RegionModel;
class NavFixModel;
class UserFixModel;
class AirportModel;
class AirwayModel;

class InMemoryNavDb : public LoadableNavDb
{
public:
    InMemoryNavDb();

public:
    // NavDatabase overrides
    int maxDensity(const world::Location &bottomLeft, const world::Location &topRight) override;
    void visitNodes(const world::Location &bottomLeft, const world::Location &topRight, NodeAcceptor callback, int filter) override;

    std::shared_ptr<Airport> findAirportByID(const std::string &id) const override;
    std::shared_ptr<Airport> findAirportByCode(const std::string &id) const override;
    std::shared_ptr<NavFix> findFixByRegionAndID(const std::string &region, const std::string &id) const override;
    std::vector<std::shared_ptr<Airport>> findAirport(const std::string &keyWord) const override;
    std::vector<std::shared_ptr<NavFix>> findFix(const std::string &id) const override;

    std::vector<NavDatabase::Connection> &getConnections(std::shared_ptr<Node> from) override;
    bool areConnected(std::shared_ptr<Node> from, const std::shared_ptr<Node> to) override;

    std::shared_ptr<Region> getRegion(const std::string &id) override;

    void clearUserFixes() override;
    void addUserFix(std::shared_ptr<UserFix> uf) override;

    NavStatus status() override {
        return load_done ? NavStatus::SUPERSEDED : (load_started ? NavStatus::RELOADING : NavStatus::ACTIVE);
    }
    std::shared_ptr<NavDatabase> latest() override { return replacement; }

    // LoadableNavDb overrides
    void setReloadStarted() override { load_started = true; }
    void setReloadCancelled() override { load_started = load_done = false; }
    void setReplacement(std::shared_ptr<NavDatabase> latest) override { replacement = latest; load_done = true; }

public:

    // used by the X-Plane NAV database loaders
    void addRegion(const std::string &code);
    void addFix(std::shared_ptr<NavFixModel> fix);

    void forEachAirport(std::function<void(std::shared_ptr<Airport>)> f);
    std::shared_ptr<RegionModel> findOrCreateRegion(const std::string &id);
    std::shared_ptr<AirportModel> findOrCreateAirport(const std::string &id);
    std::shared_ptr<AirwayModel> findOrCreateAirway(const std::string &name, AirwayLevel lvl);
    void connectTo(std::shared_ptr<Node> from, std::shared_ptr<NodeLink> via, std::shared_ptr<Node> to);

    void buildGeoIndex();

private:
    void addToGeoIndex(std::shared_ptr<Node> n);

private:
    bool indexingComplete { false };

    // Unique IDs
    std::map<std::string, std::shared_ptr<RegionModel>> regions;
    std::map<std::string, std::shared_ptr<AirportModel>> airports;

    // NAV fixes are unique only within region, user fixes are unique
    std::multimap<std::string, std::shared_ptr<NavFixModel>> navFixes;
    std::map<std::string, std::shared_ptr<UserFixModel>> userFixes;

    // Unique within airway level
    std::multimap<std::string, std::shared_ptr<AirwayModel>> airways;

    // To search by location
    std::map<std::pair<int, int>, NodeList> allNodes;

    // Connections between nodes (airports, heliports, runways, fixes)
    std::map<std::shared_ptr<Node>, std::vector<NavDatabase::Connection>> connections;
    std::vector<NavDatabase::Connection> noConnection;

private:
    // To support reloading and hot-swapping
    std::atomic_bool load_started { false };
    std::atomic_bool load_done { false };
    std::shared_ptr<NavDatabase> replacement;

};

} /* namespace navdb */

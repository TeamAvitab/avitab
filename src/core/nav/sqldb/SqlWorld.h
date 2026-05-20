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

#include "Navigation.h"
#include <future>
#include <mutex>

namespace sqlnav {

class SqlLoadManager;

class SqlWorld : public navdb::NavDatabase {
public:

    SqlWorld(std::shared_ptr<SqlLoadManager> db);
    SqlWorld() = delete;
    virtual ~SqlWorld();

    // navdb::NavDatabase overrides
    virtual int maxDensity(const world::Location &bottomLeft, const world::Location &topRight) override;
    virtual void visitNodes(const world::Location &bottomLeft, const world::Location &topRight, NodeAcceptor calllback, int filter) override;

    virtual std::shared_ptr<navdb::Airport> findAirportByID(const std::string &id) const override;
    virtual std::shared_ptr<navdb::Airport> findAirportByCode(const std::string &id) const override;
    virtual std::shared_ptr<navdb::Fix> findFixByRegionAndID(const std::string &region, const std::string &id) const override;
    virtual std::vector<std::shared_ptr<navdb::Airport>> findAirport(const std::string &keyWord) const override;

    virtual std::vector<Connection> &getConnections(std::shared_ptr<navdb::Node> from) override;
    virtual bool areConnected(std::shared_ptr<navdb::Node> from, const std::shared_ptr<navdb::Node> to) override;

    virtual std::shared_ptr<navdb::NamedNavItem> getRegion(const std::string &code) override;

    // SqlWorld extensions
    void addAirport(std::shared_ptr<navdb::Airport> a);
    void shutdown();

protected:
    void backgroundLoader();
    void addNodeToArea(int lonx_idx, int laty_idx, std::shared_ptr<navdb::Node> node);

private:
    // weak pointer prevents circular referencing to this objects owner
    std::weak_ptr<SqlLoadManager> loadManager;

    // Regions indexed by their codes
    std::map<std::string, std::shared_ptr<navdb::Region>> regions;

    // A background thread is used to load NAV items from the SQL database. The in-memory
    // cache of these nodes is protected from concurrent access by this mutex. In general
    // the background task will only hold the mutex for short periods to update the collections.
    // The foreground thread (apps) will claim the mutex for the duration of any API calls
    // to obtain node information, giving it priority.
    std::mutex navStateGuard;

    // Cache of NavNodes in each lon/lat area on the globe
    std::map<std::pair<int, int>, std::vector<std::shared_ptr<navdb::Node>>> areaNodes;
    // If the map has an entry for an area, false means it is being loaded, true means it is available.
    std::map<std::pair<int, int>, bool> areaCached;
    // Non-null when an area is being loaded asynchronously, references the lon/lat pair being loaded.
    std::unique_ptr<std::pair<int, int>> backgroundLoadArea;

    // Connections between nodes (airports, heliports, runways, fixes)
    std::map<std::shared_ptr<navdb::Node>, std::vector<navdb::NavDatabase::Connection>> connections;
    std::vector<navdb::NavDatabase::Connection> noConnection;

    // used to synchronise with the background worker
    std::future<void> asyncLoaderState;
    std::mutex backgroundLoaderGuard;
    std::condition_variable backgroundLoadControl;
};

}

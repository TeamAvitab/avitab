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
#include <stdexcept>
#include <algorithm>
#include "RouteFinder.h"
#include "Route.h"
#include "Logger.h"

namespace navdb {

RouteFinder::RouteFinder(std::shared_ptr<NavDatabase> w, NodePtr d, NodePtr a, AirwayLevel l) :
    world(w), departure(d), arrival(a), airwayLevel(l)
{
}

std::shared_ptr<Route> RouteFinder::find() {
    logger::verbose("Searching route from %s to %s", departure->getIdent().c_str(), arrival->getIdent().c_str());
    directDistance = departure->getLocation().surfaceDistanceTo(arrival->getLocation());

    auto from = departure;
    auto goal = arrival;

    // Init
    closedSet.clear();
    openSet.clear();
    openSet.insert(from);
    cameFrom.clear();
    gScore.clear();

    // The cost from start to start is zero
    gScore[from] = 0;
    fScore[from] = minCostHeuristic(from, goal);

    while (!openSet.empty()) {
        NodePtr current = getLowestOpen();
        if (current == goal) {
            logger::verbose("Route found");
            auto rp = reconstructPath(goal);
            return std::make_shared<Route>(rp);
        }

        openSet.erase(current);
        closedSet.insert(current);

        auto &neighbors = world->getConnections(current);
        for (auto neighborConn: neighbors) {
            auto &edge = std::get<0>(neighborConn);
            auto &neighbor = std::get<1>(neighborConn);
            if (!edge || !neighbor) {
                continue;
            }

            if (!checkEdge(edge, neighbor)) {
                continue;
            }

            if (closedSet.find(neighbor) != closedSet.end()) {
                continue;
            }

            if (openSet.find(neighbor) == openSet.end()) {
                openSet.insert(neighbor);
            }

            double tentativeGScore = getGScore(current) + cost(current, Leg(edge, neighbor));
            if (tentativeGScore > getGScore(neighbor)) {
                continue;
            }

            cameFrom[neighbor] = Leg(edge, current);
            gScore[neighbor] = tentativeGScore;
            fScore[neighbor] = getGScore(neighbor) + minCostHeuristic(neighbor, goal);
        }
    }

    logger::verbose("No route found");
    throw std::runtime_error("No route found");
}

std::vector<Route::Leg> RouteFinder::reconstructPath(NodePtr &lastFix) {
    logger::info("Backtracking route...");
    std::vector<Route::Leg> res;
    std::vector<world::Location> locations;

    auto cur = cameFrom[lastFix];
    res.push_back(Route::Leg(cur.second, cur.first, lastFix));

    auto it = cameFrom.begin();
    while ((it = cameFrom.find(cur.second)) != cameFrom.end()) {
        cur = cameFrom[it->first];
        res.push_back(Route::Leg(cur.second, cur.first, it->first));
    }

    std::reverse(std::begin(res), std::end(res));

    return res;
}

bool RouteFinder::checkEdge(const EdgePtr &via, const NodePtr &to) const {
    if (via->isProcedure()) {
        // We only allow SIDs, STARs etc. if they are start or end of the route.
        // This prevents routes that use SIDs and STARs of other airports as waypoints
        return world->areConnected(departure, to) || (to == arrival);
    } else {
        // Normal airways are allowed if their level matches the desired level
        return via->supportsLevel(airwayLevel);
    }
}

Route::RouteNode RouteFinder::getLowestOpen() {
    Route::RouteNode res = nullptr;
    double best = std::numeric_limits<double>::infinity();

    for (Route::RouteNode f: openSet) {
        auto fScoreIt = fScore.find(f);
        if (fScoreIt != fScore.end()) {
            if (fScoreIt->second < best) {
                res = fScoreIt->first;
                best = fScoreIt->second;
            }
        }
    }
    return res;
}

double RouteFinder::minCostHeuristic(const NodePtr &a, const NodePtr &b) {
    // the minimum cost is a direct line
    return a->getLocation().surfaceDistanceTo(b->getLocation());
}

double RouteFinder::cost(NodePtr &a, const Leg &l) {
    // the actual cost can have penalties later
    double penalty = 0;
    auto it = cameFrom.find(a);
    if (it != cameFrom.end()) {
        if (it->second.first != l.first) {
            penalty += airwayChangePenalty * directDistance;
        }
    }

    return minCostHeuristic(a, l.second) + penalty;
}

double RouteFinder::getGScore(NodePtr &f) {
    auto it = gScore.find(f);
    if (it == gScore.end()) {
        return std::numeric_limits<double>::infinity();
    }
    return it->second;
}

} /* namespace navdb */

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

#include <vector>
#include <memory>
#include <set>
#include <map>
#include <functional>
#include <cmath>
#include "Navigation.h"
#include "Route.h"

namespace navdb {

class RouteFinder {
public:
    using EdgePtr = std::shared_ptr<NodeLink>;
    using NodePtr = std::shared_ptr<Node>;

    RouteFinder(std::shared_ptr<NavDatabase> world, NodePtr dep, NodePtr arr, AirwayLevel level);
    std::shared_ptr<Route> find();

    RouteFinder() = delete;

private:
    std::shared_ptr<NavDatabase> const world;
    NodePtr const departure;
    NodePtr const arrival;
    AirwayLevel const airwayLevel;

    double directDistance = 0;
    float airwayChangePenalty = 0;

    using Leg = std::pair<EdgePtr, NodePtr>;

    std::set<NodePtr> closedSet;
    std::set<NodePtr> openSet;
    std::map<NodePtr, Leg> cameFrom;
    std::map<NodePtr, double> gScore;
    std::map<NodePtr, double> fScore;

    bool checkEdge(const EdgePtr &via, const NodePtr &to) const;
    NodePtr getLowestOpen();
    double minCostHeuristic(const NodePtr &a, const NodePtr &b);
    double cost(NodePtr &a, const Leg &dir);
    double getGScore(NodePtr &f);
    std::vector<Route::Leg> reconstructPath(NodePtr &lastFix);
};

} /* namespace navdb */

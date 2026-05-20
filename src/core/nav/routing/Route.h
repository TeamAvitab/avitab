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

#include <memory>
#include <functional>
#include <cmath>
#include "Navigation.h"
#include "WorldGeometry.h"

namespace navdb {

class Route {
public:
    using RouteNode = std::shared_ptr<Node>;
    using RouteLeg = std::shared_ptr<NodeLink>;

    struct Leg {
        std::shared_ptr<Node> from;
        std::shared_ptr<NodeLink> via;
        std::shared_ptr<Node> to;
        world::Trajectory path;
        float magDecl;
        float distanceNm;

        Leg(RouteNode from, RouteLeg via, RouteNode to, float magDecl = 0);
    };

    using RouteIterator = std::function<void (const RouteLeg, const RouteNode)>;
    using LegIterator = std::function<void (const RouteNode, const RouteLeg, const RouteNode, float distanceNm, float departureBearing, float magVar)>;

    Route(std::vector<Route::Leg> &route);
    size_t size() const { return legs.size(); }
    std::shared_ptr<Node> front() { return legs.front().from; }
    std::shared_ptr<Node> back() { return legs.back().to; }

    void applyMagDecls(std::vector<float> &mgdcls);

    void iterateRouteShort(RouteIterator f) const;
    void iterateLegs(LegIterator f) const;

    double getDirectDistance() const;
    double getRouteDistance() const;

private:
    std::vector<Route::Leg> legs;
};

} /* namespace navdb */

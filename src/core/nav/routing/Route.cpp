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
#include <limits>
#include <cassert>
#include "Route.h"
#include "Logger.h"

namespace navdb {

Route::Leg::Leg(RouteNode f, RouteLeg v, RouteNode t, float md)
:   from(f), via(v), to(t), magDecl(md)
{
    path = world::Trajectory(from->getLocation(), to->getLocation());
    distanceNm = (from->getLocation().surfaceDistanceTo(to->getLocation()) / 1000) * world::KM_TO_NM;
}

Route::Route(std::vector<Route::Leg> &r)
:   legs(r)
{
    if (legs.empty()) {
        throw std::runtime_error(std::string("route has no segments"));
    }
}

void Route::applyMagDecls(std::vector<float> &mgdcls)
{
    assert(legs.size() == mgdcls.size());
    auto li = legs.begin();
    auto mi = mgdcls.begin();
    while (li != legs.end()) {
        li->magDecl = *mi;
        ++li; ++mi;
    }
}

// this iterator collapses adjacent legs along the same airway into
// a single callback - although this is arguably wrong for 2 successive
// DRCT legs, so might need some testing and rethinking

void Route::iterateRouteShort(RouteIterator f) const {
    std::shared_ptr<NodeLink> currentEdge = nullptr;
    std::shared_ptr<Node> nextNode = legs.front().from;

    // start with null leg and start node
    f(nullptr, nextNode);

    for (auto &l: legs) {
        if (!currentEdge) {
            currentEdge = l.via;
        }

        if (currentEdge != l.via) {
            f(currentEdge, nextNode);
            currentEdge = l.via;
        }

        nextNode = l.to;
    }

    f(currentEdge, nextNode);
}

void Route::iterateLegs(LegIterator f) const {
    for (auto &l: legs) {
        auto d = l.path.hdgDegrees();
        f(l.from, l.via, l.to, l.distanceNm, d, d + l.magDecl);
    }
}

double Route::getDirectDistance() const {
    return legs.front().from->getLocation().surfaceDistanceTo(legs.back().to->getLocation());
}

double Route::getRouteDistance() const {
    double distance = 0;

    std::shared_ptr<Node> prevNode = legs.front().from;

    for (auto &l: legs) {
        distance += prevNode->getLocation().surfaceDistanceTo(l.to->getLocation());
        prevNode = l.to;
    }

    return distance;
}

} /* namespace navdb */

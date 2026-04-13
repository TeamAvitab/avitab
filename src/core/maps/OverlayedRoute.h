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

#include "OverlayHelper.h"
#include "core/world/graph/NavNode.h"
#include "core/world/routing/Route.h"
#include "core/libimg/Image.h"
#include "core/Logger.h"

namespace maps {

class OverlayedRoute {
public:
    OverlayedRoute(IOverlayHelper *h);
    void draw(std::shared_ptr<world::Route> route);

private:
    IOverlayHelper * const overlayHelper;
    std::map<uint32_t, std::shared_ptr<img::Image>> routeAnnotationCache;

    void drawLeg(world::Location &from, world::Location &to, double distance,
                 double trueBearing, double magneticBearing);
    std::pair<int, int> drawLeg(int x0, int y0, int x1, int y1);
    std::shared_ptr<img::Image> createRouteAnnotation(int distance, int trueBearing, int magBearing);
};

} /* namespace maps */

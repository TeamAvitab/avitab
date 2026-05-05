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

#include <string>
#include "WorldGeometry.h"
#include "graph/NavNode.h"

namespace world {

class Fix;

class Heliport: public NavNode {
public:
    Heliport(const std::string &name);
    void setLocation(const world::Location &loc);
    void setLength(const float &l);
    void setWidth(const float &w);

    float getLength() const;
    float getWidth() const;

    const std::string &getID() const override;
    const world::Location &getLocation() const override;
    bool isAirport() const override { return true; }
    bool isFix() const override { return false; }

private:
    std::string name;
    world::Location location;
    float length = std::numeric_limits<float>::quiet_NaN(); // meters
    float width = std::numeric_limits<float>::quiet_NaN(); // meters
};

} /* namespace world */

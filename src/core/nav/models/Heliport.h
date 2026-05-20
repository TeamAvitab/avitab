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
#include "Navigation.h"

namespace navdb {

class HeliportModel : public Heliport
{
public:
    HeliportModel(const std::string &id) : name(id) { }
    const std::string &getIdent() const override { return name; }
    const world::Location& getLocation() const override { return location; }
    void setLocation(const world::Location &loc) { location = loc; }
    float getLength() const override { return length; }
    void setLength(float l) { length = l; }
    float getWidth() const override { return width; }
    void setWidth(float w) { width = w; }
private:
    std::string name;
    world::Location location;
    float length = std::numeric_limits<float>::quiet_NaN(); // meters
    float width = std::numeric_limits<float>::quiet_NaN(); // meters
};

} /* namespace navdb */

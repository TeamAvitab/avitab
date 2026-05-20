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
#include <memory>
#include <limits>
#include "Navigation.h"
#include "WorldGeometry.h"

namespace navdb {

class RunwayModel : public Runway
{
public:
    RunwayModel(const std::string &id) : name(id) { }

    const std::string &getIdent() const override { return name; }
    const world::Location& getLocation() const override { return location; }
    float getHeading() const override { return heading; }
    float getLength() const override { return length; }
    float getWidth() const override { return width; }
    float getElevation() const override { return elevation; }
    bool hasHardSurface() const override;
    bool isWater() const override;
    SurfaceMaterial getSurfaceType() const override { return surfaceType; }
    const char *getSurfaceTypeDescription() const override;
    std::shared_ptr<NavFix> getNavaidILS() const override { return ils; }

    void rename(const std::string &newName) { name = newName; }
    void setLocation(const world::Location &loc) { location = loc; }
    void setHeading(float h) { heading = h; }
    void setLength(float l) { length = l; }
    void setWidth(float w) { width = w; }
    void setElevation(float e) { elevation = e; }
    void setSurfaceType(SurfaceMaterial s) { surfaceType = s; }
    void setNavaidILS(std::shared_ptr<NavFix> fix);

private:
    std::string name;
    world::Location location;
    float heading = std::numeric_limits<float>::quiet_NaN(); // degrees
    float length = std::numeric_limits<float>::quiet_NaN(); // meters
    float width = std::numeric_limits<float>::quiet_NaN(); // meters
    float elevation = std::numeric_limits<float>::quiet_NaN(); // feet MSL
    SurfaceMaterial surfaceType;
    std::shared_ptr<NavFix> ils;
};

} /* namespace navdb */

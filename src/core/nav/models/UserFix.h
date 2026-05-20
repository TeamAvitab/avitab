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
#include "WorldGeometry.h"
#include "Region.h"

namespace navdb {

class UserFixModel : public UserFix
{
public:
    UserFixModel(UserFixType t, std::string n, std::string id, world::Location l);
    const std::string & getIdent() const override { return ident; }
    const std::string & getName() const override { return name; }
    const world::Location & getLocation() const override { return location; }
    std::shared_ptr<Region> getRegion() const override  { return userRegion; }
    UserFixType getType() const override { return type; }
    void update(UserFixType t, std::string n, world::Location l) { type = t; name = n; location = l;}
private:
    UserFixType type = UserFixType::NONE;
    std::string name;
    std::string ident;
    world::Location location;
    static std::shared_ptr<RegionModel> userRegion;
};

} /* namespace navdb */

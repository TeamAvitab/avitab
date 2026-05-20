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
#include "Navigation.h"

namespace navdb {

class AirwayModel : public Airway
{
public:
    AirwayModel(const std::string &id, AirwayLevel l) : name(id), level(l) { }

    const std::string &getIdent() const override { return name; }
    void setIdent(const std::string &id) { name = id; }

    bool supportsLevel(AirwayLevel l) const override { return level == l; }

    AirwayLevel getLevel() const override { return level; }
    void setLevel(AirwayLevel l) { level = l; }

private:
    std::string name;
    AirwayLevel level;
};

} /* namespace navdb */

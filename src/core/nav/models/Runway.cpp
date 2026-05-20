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
#include "Runway.h"

namespace navdb {

bool RunwayModel::hasHardSurface() const{
    return ((surfaceType == SurfaceMaterial::ASPHALT) ||
            (surfaceType == SurfaceMaterial::BITUMINOUS) ||
            (surfaceType == SurfaceMaterial::CONCRETE) ||
            (surfaceType == SurfaceMaterial::ICE) ||
            (surfaceType == SurfaceMaterial::LAKEBED) ||
            (surfaceType == SurfaceMaterial::TARMAC) ||
            (surfaceType == SurfaceMaterial::CUSTOM));
}

bool RunwayModel::isWater() const{
    return (surfaceType == SurfaceMaterial::WATER);
}

const char * RunwayModel::getSurfaceTypeDescription() const {
    switch(surfaceType) {
    case SurfaceMaterial::ASPHALT:      return "Asphalt";
    case SurfaceMaterial::BITUMINOUS:   return "Bituminous";
    case SurfaceMaterial::CONCRETE:     return "Concrete";
    case SurfaceMaterial::CORAL:        return "Coral";
    case SurfaceMaterial::DIRT:         return "Dirt";
    case SurfaceMaterial::GRASS:        return "Grass";
    case SurfaceMaterial::GRAVEL:       return "Gravel";
    case SurfaceMaterial::ICE:          return "Ice";
    case SurfaceMaterial::LAKEBED:      return "Lakebed";
    case SurfaceMaterial::SAND:         return "Sand";
    case SurfaceMaterial::SNOW:         return "Snow";
    case SurfaceMaterial::TARMAC:       return "Tarmacadam";
    case SurfaceMaterial::WATER:        return "Water";
    default:                            return "Unknown";
    }
}

void RunwayModel::setNavaidILS(std::shared_ptr<NavFix> fix) {
    if (!fix->getILSLocalizer()) {
        throw std::runtime_error("Adding ILS fix without ILS data");
    }

    ils = fix;
}

} /* namespace navdb */

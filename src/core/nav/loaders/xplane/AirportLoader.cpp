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
#include <cmath>
#include <limits>
#include "AirportLoader.h"
#include "NavLoaderXp.h"
#include "nav/InMemNavDb.h"
#include "nav/parsers/xplane/AirportParser.h"
#include "nav/models/Airport.h"
#include "nav/models/Heliport.h"
#include "nav/models/Runway.h"
#include "nav/models/Region.h"
#include "Logger.h"

namespace xdata {

inline navdb::SurfaceMaterial mapToSurfaceMaterial(AirportData::SurfaceCode c) {
    static const navdb::SurfaceMaterial runwaySurfaceMap[] = {
        navdb::SurfaceMaterial::UNKNOWN,    // [0]
        navdb::SurfaceMaterial::ASPHALT,    // [1]
        navdb::SurfaceMaterial::CONCRETE,   // [2]
        navdb::SurfaceMaterial::GRASS,      // [3]
        navdb::SurfaceMaterial::DIRT,       // [4]
        navdb::SurfaceMaterial::GRAVEL,     // [5]
        navdb::SurfaceMaterial::UNKNOWN,    // [6]
        navdb::SurfaceMaterial::UNKNOWN,    // [7]
        navdb::SurfaceMaterial::UNKNOWN,    // [8]
        navdb::SurfaceMaterial::UNKNOWN,    // [9]
        navdb::SurfaceMaterial::UNKNOWN,    // [10]
        navdb::SurfaceMaterial::UNKNOWN,    // [11]
        navdb::SurfaceMaterial::LAKEBED,    // [12]
        navdb::SurfaceMaterial::WATER,      // [13]
        navdb::SurfaceMaterial::SNOW,       // [14]
        navdb::SurfaceMaterial::CUSTOM,     // [15]
        navdb::SurfaceMaterial::UNKNOWN,    // [16]
        navdb::SurfaceMaterial::UNKNOWN,    // [17]
        navdb::SurfaceMaterial::UNKNOWN,    // [18]
        navdb::SurfaceMaterial::UNKNOWN,    // [19]
        navdb::SurfaceMaterial::ASPHALT,    // [20]
        navdb::SurfaceMaterial::ASPHALT,    // [21]
        navdb::SurfaceMaterial::ASPHALT,    // [22]
        navdb::SurfaceMaterial::ASPHALT,    // [23]
        navdb::SurfaceMaterial::ASPHALT,    // [24]
        navdb::SurfaceMaterial::ASPHALT,    // [25]
        navdb::SurfaceMaterial::ASPHALT,    // [26]
        navdb::SurfaceMaterial::ASPHALT,    // [27]
        navdb::SurfaceMaterial::ASPHALT,    // [28]
        navdb::SurfaceMaterial::ASPHALT,    // [29]
        navdb::SurfaceMaterial::ASPHALT,    // [30]
        navdb::SurfaceMaterial::ASPHALT,    // [31]
        navdb::SurfaceMaterial::ASPHALT,    // [32]
        navdb::SurfaceMaterial::ASPHALT,    // [33]
        navdb::SurfaceMaterial::ASPHALT,    // [34]
        navdb::SurfaceMaterial::ASPHALT,    // [35]
        navdb::SurfaceMaterial::ASPHALT,    // [36]
        navdb::SurfaceMaterial::ASPHALT,    // [37]
        navdb::SurfaceMaterial::ASPHALT,    // [38]
        navdb::SurfaceMaterial::UNKNOWN,    // [39]
        navdb::SurfaceMaterial::UNKNOWN,    // [40]
        navdb::SurfaceMaterial::UNKNOWN,    // [41]
        navdb::SurfaceMaterial::UNKNOWN,    // [42]
        navdb::SurfaceMaterial::UNKNOWN,    // [43]
        navdb::SurfaceMaterial::UNKNOWN,    // [44]
        navdb::SurfaceMaterial::UNKNOWN,    // [45]
        navdb::SurfaceMaterial::UNKNOWN,    // [46]
        navdb::SurfaceMaterial::UNKNOWN,    // [47]
        navdb::SurfaceMaterial::UNKNOWN,    // [48]
        navdb::SurfaceMaterial::UNKNOWN,    // [49]
        navdb::SurfaceMaterial::CONCRETE,   // [50]
        navdb::SurfaceMaterial::CONCRETE,   // [51]
        navdb::SurfaceMaterial::CONCRETE,   // [52]
        navdb::SurfaceMaterial::CONCRETE,   // [53]
        navdb::SurfaceMaterial::CONCRETE,   // [54]
        navdb::SurfaceMaterial::CONCRETE,   // [55]
        navdb::SurfaceMaterial::CONCRETE,   // [56]
        navdb::SurfaceMaterial::CONCRETE    // [57]
    };
    if (c > AirportData::SurfaceCode::SC_CONCRETE_DARK) {
        return navdb::SurfaceMaterial::UNKNOWN;
    }
    return runwaySurfaceMap[(size_t)c];
}

AirportLoader::AirportLoader(XplaneNavDataLoader * mgr):
    loadMgr(mgr),
    world(mgr->getDatabase())
{
}

void AirportLoader::load(const std::filesystem::path& file) const {
    AirportParser parser(file);
    parser.setAcceptor([this] (const AirportData &data) {
        try {
            onAirportLoaded(data);
        } catch (const std::exception &e) {
            logger::warn("Can't parse airport %s: %s", data.id.c_str(), e.what());
        }
        if (loadMgr->shouldCancelLoading()) {
            throw std::runtime_error("Cancelled");
        }
    });
    parser.loadAirports();
}

void AirportLoader::onAirportLoaded(const AirportData& port) const {
    if (std::isnan(port.latitude) || std::isnan(port.longitude)) {
        if (port.runways.empty() && port.heliports.empty()) {
            logger::warn("Airport %s has no location or landing points, discarding", port.id.c_str());
            return;
        }
    }

    auto airport = std::dynamic_pointer_cast<navdb::AirportModel>(world->findAirportByID(port.id));
    if (airport) {
        //Airport (custom scenery) already exists so don't re-construct it, but we do want to:
        patchCustomSceneryRunwaySurfaces(port, airport);
        return;
    }

    airport = world->findOrCreateAirport(port.id);
    airport->setName(port.name);
    airport->setElevation(port.elevation);

    if (!port.region.empty()) {
        auto region = world->findOrCreateRegion(port.region);
        airport->setRegion(region);
        if (!port.country.empty()) {
            region->setName(port.country);
        }
    }

    if (!port.localCode.empty()) {
        airport->setLocalCode(port.localCode);
    }

    if (!port.faaCode.empty()) {
        airport->setFAACode(port.faaCode);
    }

    if (!port.icaoCode.empty()) {
        airport->setICAOCode(port.icaoCode);
    }

    for (auto &entry: port.frequencies) {
        int places = 2;
        int code = entry.code;
        // to support 8 kHz channel spacing
        if (code > 1000) {
            places = 3;
            code -= 1000;
        }

        navdb::RadioFrequency frq(entry.frq, places, navdb::FrequencyUnit::MHZ, entry.desc);
        switch (code) {
        case 50: airport->addATCFrequency(navdb::ATCserviceType::RECORDED, frq); break;
        case 51: airport->addATCFrequency(navdb::ATCserviceType::UNICOM, frq); break;
        case 52: airport->addATCFrequency(navdb::ATCserviceType::CLD, frq); break;
        case 53: airport->addATCFrequency(navdb::ATCserviceType::GND, frq); break;
        case 54: airport->addATCFrequency(navdb::ATCserviceType::TWR, frq); break;
        case 55: airport->addATCFrequency(navdb::ATCserviceType::APP, frq); break;
        case 56: airport->addATCFrequency(navdb::ATCserviceType::DEP, frq); break;
        }
    }

    for (auto &entry: port.heliports) {
        auto heliport = std::make_shared<navdb::HeliportModel>(entry.name);
        heliport->setLocation(world::Location::fromGCS(entry.latitude, entry.longitude));
        heliport->setLength(entry.length);
        heliport->setWidth(entry.width);
        airport->addHeliport(heliport);
    }

    for (auto &entry: port.runways) {
        float heading = std::numeric_limits<float>::quiet_NaN();
        float length = std::numeric_limits<float>::quiet_NaN();
        if (entry.ends.size() == 2) {
            auto &end1 = entry.ends[0];
            auto &end2 = entry.ends[1];
            auto end1Loc = world::Location::fromGCS(end1.latitude, end1.longitude);
            auto end2Loc = world::Location::fromGCS(end2.latitude, end2.longitude);
            world::Trajectory tr(end1Loc, end2Loc);
            heading = tr.hdgDegrees();
            length = end1Loc.surfaceDistanceTo(end2Loc) - end1.displace - end2.displace;
        } else {
            LOG_WARN("%s has runway with %d ends!", port.id.c_str(), entry.ends.size());
        }

        std::shared_ptr<navdb::RunwayModel> end0;
        for (auto end = entry.ends.begin(); end != entry.ends.end(); ++end) {
            auto rwy = std::make_shared<navdb::RunwayModel>(end->name);
            rwy->setLocation(world::Location::fromGCS(end->latitude, end->longitude));
            rwy->setWidth(entry.width);
            rwy->setSurfaceType(mapToSurfaceMaterial(entry.surfaceTypeCode));
            if (!std::isnan(heading)) {
                rwy->setHeading(end == entry.ends.begin() ? heading : std::fmod(heading + 180.0, 360.0));
            }
            if (!std::isnan(length)) {
                rwy->setLength(length);
            }
            airport->addRunway(rwy);
            if (entry.ends.size() == 2) {
                if (end == entry.ends.begin()) {
                    end0 = rwy;
                } else {
                    airport->addRunwayEnds(end0, rwy);
                    LOG_VERBOSE(0,"%s runway pair %s & %s", port.id.c_str(), end0->getName().c_str(), rwy->getName().c_str());
                }
            }
        }
    }

    // the location of the airport (for Avitab's purposes) is the centre of the area
    // enclosing the runways, or if no runways then the supplied coordinates, or if no
    // coordinates then the first heliport.
    if (port.runways.size() > 0) {
        airport->setLocation(airport->getCornerSW().areaCenter(airport->getCornerNE()));
    } else if (!std::isnan(port.latitude) && !std::isnan(port.longitude)) {
        auto loc = world::Location::fromGCS(port.latitude, port.longitude);
        airport->setLocation(loc);
    } else if (port.heliports.size() > 0) {
        auto &h = port.heliports[0];
        airport->setLocation(world::Location::fromGCS(h.latitude, h.longitude));
    }
}

void AirportLoader::patchCustomSceneryRunwaySurfaces(const AirportData& defaultAirportData, std::shared_ptr<navdb::AirportModel> customAirport) const {
    // Scenery, typically custom, may use SurfaceTypeCode 15 (= transparent) for runways. Although 15 is described
    // in the apt.dat spec as "hard", this results is the misclassification of custom scenery grass airstrips as airports.
    // To get better surface types for avitab, we attempt to patch good runway surface type codes from a later loaded
    // default scenery back into the airport we first created (typically from custom scenery).
    // Note that 15 is also used in default scenery, but there's not a lot we can do about that.

    // Gather map of runway IDs and their surface types from airport in the default scenery being loaded
    std::map<std::string, AirportData::SurfaceCode> defaultRwySurfaceCode;
    for (auto &entry: defaultAirportData.runways) {
        for (auto end = entry.ends.begin(); end != entry.ends.end(); ++end) {
            if (entry.surfaceTypeCode != AirportData::SurfaceCode::SC_TRANSPARENT_SURFACE) {
                defaultRwySurfaceCode[end->name] = entry.surfaceTypeCode;
            }
        }
    }

    // Patch surface type from airport in default scenery into the airport that came from custom scenery loaded earlier
    customAirport->forEachRunway([customAirport, defaultRwySurfaceCode] (std::shared_ptr<navdb::Runway> customRwy) {
        auto rwy = std::dynamic_pointer_cast<navdb::RunwayModel>(customRwy);
        auto runwayID = rwy->getName();
        if (rwy->getSurfaceType() == navdb::SurfaceMaterial::CUSTOM) {
            auto defaultSurface = defaultRwySurfaceCode.find(runwayID);
            if (defaultSurface != defaultRwySurfaceCode.end()) {
                 rwy->setSurfaceType(mapToSurfaceMaterial(defaultSurface->second));
                 LOG_INFO(0, "For custom scenery %s, rwy %s, transparent surface overridden with %s from default scenery",
                     customAirport->getIdent().c_str(), runwayID.c_str(), rwy->getSurfaceTypeDescription());
            }
        }
    });
}

} /* namespace xdata */

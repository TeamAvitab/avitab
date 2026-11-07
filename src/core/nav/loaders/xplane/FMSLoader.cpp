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
#include <algorithm>
#include <cassert>
#include "FMSLoader.h"
#include "nav/parsers/xplane/FMSParser.h"
#include "nav/models/UserFix.h"
#include "nav/routing/Route.h"
#include "WorldGeometry.h"
#include "Logger.h"

namespace xdata {

FMSLoader::FMSLoader(navdb::NavDatabase *navdb):
    world(navdb)
{
}

std::shared_ptr<navdb::Route> FMSLoader::load(const std::filesystem::path &fmsFilename) {
    logger::info("Loading %s", fmsFilename.string().c_str());
    FMSParser parser(fmsFilename);
    waypoints.clear();
    parser.setAcceptor([this] (const FlightPlanNodeData &data) {
        try {
            onFMSLoaded(data);
        } catch (const std::exception &e) {
            logger::warn("Can't parse FMS %d %s: %s", data.lineNum, data.line.c_str(), e.what());
        }
    });
    parser.loadFMS();

    waypoints.erase(std::unique(waypoints.begin(), waypoints.end()), waypoints.end());

    std::stringstream ss;
    for (auto w: waypoints) {
        ss << w.node->getIdent() << " ";
    }
    logger::info("Using waypoints: %s", ss.str().c_str());

    // convert the waypoints to a Route object.
    std::vector<navdb::Route::Leg> legs;
    auto wps = waypoints.begin();
    auto wpe = wps + 1;
    // each Leg of the Route object has both start and finish, these are adjacent entries in the waypoints list
    while (wpe != waypoints.end()) {
        // TODO - look up the via/airway and insert it into the route
        legs.push_back(navdb::Route::Leg(wps->node, nullptr, wpe->node));
        ++wps; ++wpe;
    }
    return std::make_shared<navdb::Route>(legs);
}

void FMSLoader::onFMSLoaded(const FlightPlanNodeData &node) {
    switch(node.type) {

        case FlightPlanNodeData::Type::CYCLE:     cycle = node.id; break;
        case FlightPlanNodeData::Type::SID:       sidName = node.id; break;
        case FlightPlanNodeData::Type::SIDTRANS:  sidTransName = node.id; break;
        case FlightPlanNodeData::Type::STAR:      starName = node.id; break;
        case FlightPlanNodeData::Type::STARTRANS: starTransName = node.id; break;
        case FlightPlanNodeData::Type::DESRWY:    arrivalRwyName = stripRWPrefix(node.id); break;
        case FlightPlanNodeData::Type::APPTRANS:  approachTransName = node.id; break;
        case FlightPlanNodeData::Type::APP:       approachName = node.id; break;
        case FlightPlanNodeData::Type::ADEP:      departureAirportName = node.id; break;
        case FlightPlanNodeData::Type::ADES:      arrivalAirportName = node.id; break;
        case FlightPlanNodeData::Type::DEPRWY:    departureRwyName = stripRWPrefix(node.id); break;

        case FlightPlanNodeData::Type::NDB:
        case FlightPlanNodeData::Type::VOR:
        case FlightPlanNodeData::Type::FIX:
        case FlightPlanNodeData::Type::UNNAMED:
            appendWaypoint(node.id, world::Location::fromGCS(node.lat, node.lon), node.via, node.alt);
            break;

        case FlightPlanNodeData::Type::AIRPORT:
            if ((node.id == departureAirportName) && !seenADEPWaypoint) {
                seenADEPWaypoint = true;
                appendDeparture();
            } else if (node.id == arrivalAirportName) {
                appendArrival();
            }
            break;

        default:
            logger::warn("Unknown FlightPlanNodeData type %d", int(node.type));
            break;
    }
}

void FMSLoader::appendDeparture() {
    departureAirport = world->findAirportByID(departureAirportName);
    if (departureAirport) {
        appendDepartureAirportOrRwy();
        appendSID();
    } else {
        logger::warn("Unknown departure airport %s", departureAirportName.c_str());
    }
}

void FMSLoader::appendArrival() {
    arrivalAirport = world->findAirportByID(arrivalAirportName);
    if (arrivalAirport) {
        appendSTAR();
        appendApproach();
        appendArrivalAirportOrRwy();
    } else {
        logger::warn("Unknown arrival airport %s", arrivalAirportName.c_str());
    }

}

void FMSLoader::appendDepartureAirportOrRwy() {
    // If we have a runway, route along it, else use airport location.
    departureRunway = departureAirport->getRunwayByName(departureRwyName);
    if (departureRunway) {
        std::string idRwySER = departureAirportName + "_" + departureRwyName + "_SER" ;
        std::string idRwyDER = departureAirportName + "_" + departureRwyName + "_DER" ;
        appendWaypoint(idRwySER, departureRunway->getLocation(), "", departureAirport->getElevationFt());
        auto oppRwy = departureAirport->getOppositeRunwayEnd(departureRunway);
        if (oppRwy) {
            appendWaypoint(idRwyDER, oppRwy->getLocation(), "", departureAirport->getElevationFt());
        }
        logger::info("Using rwy %s for %s departure", departureRunway->getIdent().c_str(),
                    departureAirport->getIdent().c_str());
    } else {
        auto w = Waypoint(departureAirport, "", departureAirport->getElevationFt());
        waypoints.push_back(w);
        if (departureRwyName.empty()) {
            logger::info("No departure runway specified");
        } else {
            logger::warn("Couldn't find runway '%s' at %s", departureRwyName.c_str(), departureAirport->getIdent().c_str());
        }
    }
}

void FMSLoader::appendSID() {
    auto sid = departureAirport->getSIDByName(sidName);
    if (sid) {
        auto wpts = sid->getWaypoints(departureRunway, sidTransName);
        if (departureRunway && !wpts.empty() && (departureRunway->getIdent() == wpts.front()->getIdent())) {
            // Any rwy in FMS already in nodes, so if in new waypoints, remove
            wpts.erase(wpts.begin());
        }
        for (auto &w: wpts) {
            waypoints.push_back(Waypoint(w));
        }
    }
}

void FMSLoader::appendSTAR() {
    auto star = arrivalAirport->getSTARByName(starName);
    if (star) {
        arrivalRwy = arrivalAirport->getRunwayByName(arrivalRwyName);
        auto wpts = star->getWaypoints(arrivalRwy, starTransName);
        if (arrivalRwy && !wpts.empty() && (arrivalRwy->getIdent() == wpts.back()->getIdent())) {
            // Any rwy in FMS already in nodes, so if in new waypoints, remove
            wpts.pop_back();
        }
        for (auto &w: wpts) {
            waypoints.push_back(Waypoint(w));
        }
    }
}

void FMSLoader::appendApproach() {
    auto app = arrivalAirport->getApproachByName(approachName);
    if (app) {
        arrivalRwy = arrivalAirport->getRunwayByName(arrivalRwyName);
        auto wpts = app->getWaypoints(arrivalRwy, approachTransName);
        if (arrivalRwy && !wpts.empty() && (arrivalRwy->getIdent() == wpts.back()->getIdent())) {
            // Any rwy in FMS already in nodes, so if in new waypoints, remove
            wpts.pop_back();
        }
        for (auto &w: wpts) {
            waypoints.push_back(Waypoint(w));
        }
    }
}

void FMSLoader::appendArrivalAirportOrRwy() {
    // If we have a runway, route along it, else use airport location.
    arrivalRwy = arrivalAirport->getRunwayByName(arrivalRwyName);
    if (arrivalRwy) {
        std::string idRwy_SER = arrivalAirportName + "_" + arrivalRwyName + "_SER";
        std::string idRwy_DER = arrivalAirportName + "_" + arrivalRwyName + "_DER";
        appendWaypoint(idRwy_SER, arrivalRwy->getLocation(), "", arrivalAirport->getElevationFt());
        auto oppRwy = arrivalAirport->getOppositeRunwayEnd(arrivalRwy);
        if (oppRwy) {
            appendWaypoint(idRwy_DER, oppRwy->getLocation(), "", arrivalAirport->getElevationFt());
        }
        logger::info("Using rwy %s for %s arrival", arrivalRwy->getIdent().c_str(),
                    arrivalAirport->getIdent().c_str());
    } else {
        waypoints.push_back(Waypoint(arrivalAirport, "", arrivalAirport->getElevationFt()));
        if (arrivalRwyName.empty()) {
            logger::info("No arrival runway specified");
        } else {
            logger::warn("Couldn't find runway '%s' at %s", arrivalRwyName.c_str(), arrivalAirport->getIdent().c_str());
        }
    }
}

void FMSLoader::appendWaypoint(const std::string &id, const world::Location &loc, const std::string &via, float alt) {
    // search NAVdb for the named item first
    std::shared_ptr<navdb::Fix> fix;
    auto fxs = world->findFix(id);
    for (auto &f : fxs) {
        if ((loc.angularDistanceTo(f->getLocation()) / world::KM_TO_RAD) < 1.0f) {
            fix = f;
            break;
        }
    }
    // if not found then create a new user fix for this waypoint
    if (!fix) {
        fix = std::make_shared<navdb::UserFixModel>(navdb::UserFixType::ROUTEFIX, id, id, loc);
    } 

    waypoints.push_back(Waypoint(fix));
}

std::string FMSLoader::stripRWPrefix(const std::string rwyID) {
    std::string rwyNum = rwyID;
    if (rwyNum.find("RW") == 0) {
        rwyNum.replace(0, 2, ""); // Strip "RW" prefix
    }
    return rwyNum;
}

} /* namespace xdata */

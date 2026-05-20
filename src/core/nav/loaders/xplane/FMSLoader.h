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

#include <memory>
#include <filesystem>
#include <filesystem>
#include "Navigation.h"

namespace navdb {
class Route;
}

namespace xdata {

class FlightPlanNodeData;

class FMSLoader {
public:
    FMSLoader(navdb::NavDatabase *navdb);
    std::shared_ptr<navdb::Route> load(const std::filesystem::path &fmsFilename);

private:
    void onFMSLoaded(const FlightPlanNodeData &node);
    void appendDeparture();
    void appendArrival();
    void appendDepartureAirportOrRwy();
    void appendSID();
    void appendSTAR();
    void appendApproach();
    void appendArrivalAirportOrRwy();
    void appendWaypoint(const std::string &id, const world::Location &loc, const std::string &via = "DRCT", float alt = 0);
    std::string stripRWPrefix(const std::string rwyID);

private:
    struct Waypoint {
        std::shared_ptr<navdb::Node> node;
        std::string via;
        float altitude;
        Waypoint(std::shared_ptr<navdb::Node> n) : node(n), via("DRCT"), altitude(0) { }
        Waypoint(std::shared_ptr<navdb::Node> n, std::string v, float a) : node(n), via(v), altitude(a) { }
        bool operator==(const Waypoint &w) const { return w.node == node; }
        Waypoint(const Waypoint&) = default;
        Waypoint& operator=(Waypoint&&) = default;
    };

private:
    navdb::NavDatabase * const world; // this is used read-only for looking up nodes
    std::vector<Waypoint> waypoints;
    std::shared_ptr<navdb::Airport> departureAirport, arrivalAirport;
    std::shared_ptr<navdb::Runway> departureRunway, arrivalRwy;
    std::string cycle;
    std::string departureAirportName, departureRwyName, sidName, sidTransName;
    std::string arrivalAirportName, arrivalRwyName, starName, starTransName;
    std::string approachName, approachTransName;

    bool seenADEPWaypoint = false;
};

} /* namespace xdata */

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

#include "Procedures.h"
#include "Logger.h"
#include <sstream>
#include <algorithm>

namespace navdb {

ProcedureOptions::ProcedureOptions(std::string id)
:   transitionId(id)
{
}

void ProcedureOptions::attachRunwayTransition(std::shared_ptr<Runway> rwy, const NodeList &nodes)
{
    if (!rwy) {
        throw std::runtime_error("Null runway passed to procedure " + transitionId);
    }
    runwayTransitions.insert(std::make_pair(rwy, nodes));
}

void ProcedureOptions::attachCommonRoute(std::shared_ptr<Node> start, const NodeList &nodes)
{
    commonRoutes.insert(std::make_pair(start, nodes));
}

void ProcedureOptions::attachEnrouteTransitions(const NodeList& nodes)
{
    enrouteTransitions.push_back(nodes);
}

std::string ProcedureOptions::toDebugString() const
{
    std::stringstream res;

    for (auto &it: runwayTransitions) {
        res << "  Runway " << it.first->getIdent() << " connects to ";
        for (auto &node: it.second) {
            res << node->getIdent() << ", ";
        }
        res << "\n";
    }

    for (auto &it: commonRoutes) {
        res << "  Common route " << it.first->getIdent() << " connects ";
        for (auto &node: it.second) {
            res << node->getIdent() << ", ";
        }
        res << "\n";
    }

    for (auto &enrt: enrouteTransitions) {
        res << "  Enroute transition ";
        for (auto &node: enrt) {
            res << node->getIdent() << ", ";
        }
        res << "\n";
    }

    return res.str();
}


NodeList SIDModel::getWaypoints(std::shared_ptr<Runway> departureRwy, std::string sidTransName) const {

    std::string runwayID = departureRwy ? departureRwy->getIdent() : "";
    if ((runwayTransitions.size() + commonRoutes.size() + enrouteTransitions.size()) == 0) {
        logger::warn("SID %s has no waypoints", getIdent().c_str());
        return NodeList();
    }

    logger::info("SID %s:\n%s", getIdent().c_str(), toDebugString().c_str());
    logger::info("Want from runway '%s' to fix '%s'", runwayID.c_str(), sidTransName.c_str());

    /* We're going to run a 3 tier (runway, common, enroute) nested loop through
     * all permutations of a full SID route. So transform the structures into vector<vector>
     * ensuring that all outer vectors have at least an empty vector inside so as to give the
     * appropriate loop a once through. If the map key NavNode is not in the vector<NavNcde>
     * then add it.
     */
    std::vector<NodeList> rr;
    std::vector<NodeList> cc;
    std::vector<NodeList> ee;
    NodeList empty;
    if (runwayTransitions.size() > 0) {
        for (auto it: runwayTransitions) {
            auto w = it.second;
            if (std::find(w.begin(), w.end(), it.first) == w.end()) {
                w.insert(w.begin(), it.first);
            }
            rr.push_back(w);
        }
    } else {
        rr.push_back(empty);
    }
    if (commonRoutes.size() > 0) {
        for (auto it: commonRoutes) {
            auto w = it.second;
            if (std::find(w.begin(), w.end(), it.first) == w.end()) {
                w.insert(w.begin(), it.first);
            }
            cc.push_back(w);
        }
    } else {
        cc.push_back(empty);
    }
    if (enrouteTransitions.size() > 0) {
        ee = enrouteTransitions;
    } else {
        ee.push_back(empty);
    }

    /* 3 tier loop through all permutations of (runway, common, enroute)
     * Check if a permutation meets the constraints of runway and enroute that may or may
     * not have been specified in the flight plan.
     * Maintain a list of common waypoints in each of the successive validated permutations.
     * This will often end up the same as one of the entries in the commonRoutes map.
     * But sometimes there may be a common route even if there are no entries in the commonRoute map.
     * And sometimes the common route returned can be larger than an entry in the commonRoute map.
     */
    std::vector<NodeList> routePermutations;
    NodeList repeatedWaypoints;
    logger::info("Possible routes are:");
    for (auto r: rr) {
        for (auto c: cc) {
            for (auto e: ee) {
                NodeList waypoints;
                waypoints.insert(waypoints.end(), r.begin(), r.end());
                waypoints.insert(waypoints.end(), c.begin(), c.end());
                waypoints.insert(waypoints.end(), e.begin(), e.end());
                auto newEnd = std::unique(waypoints.begin(), waypoints.end());
                waypoints.erase(newEnd, waypoints.end());

                if (!waypoints.empty() && waypoints.front()->isRunway() &&
                    departureRwy && (waypoints.front() != departureRwy)) {
                    // Specified departure runway doesn't match - unsuitable
                    continue;
                }
                if (!waypoints.empty() && !sidTransName.empty() && (waypoints.back()->getIdent() != sidTransName)) {
                    // Specified transition doesn't match - unsuitable
                    continue;
                }
                if (repeatedWaypoints.empty()) {
                    // Initialise
                    repeatedWaypoints = waypoints;
                } else {
                    // Filter the list of repeated waypoints using new valid permutation
                    auto iter = std::remove_if(std::begin(repeatedWaypoints), std::end(repeatedWaypoints), [&waypoints] (const auto &a) -> bool {
                        return (find(waypoints.begin(), waypoints.end(), a) == waypoints.end());
                    });
                    repeatedWaypoints.erase(iter, std::end(repeatedWaypoints));
                }
                routePermutations.push_back(waypoints);

                std::stringstream ss;
                for (auto w: waypoints) {
                    ss << w->getIdent() << " ";
                }
                logger::info(" %s", ss.str().c_str());
            }
        }
    }

    switch(routePermutations.size()) {
    case 0:  logger::warn("No waypoints found");
             return NodeList();
    case 1:  return routePermutations.front();
    default: std::stringstream ss;
             for (auto w: repeatedWaypoints) {
                 ss << w->getIdent() << " ";
             }
             logger::info("Using common waypoints: %s", ss.str().c_str());
             return repeatedWaypoints;
    }
}

void SIDModel::iterate(std::function<void(std::shared_ptr<Runway>, std::shared_ptr<NavFix>)> f) const {
    std::map<std::shared_ptr<Runway>, std::shared_ptr<Node>> runwayToWaypoint;
    std::map<std::shared_ptr<Node>, std::shared_ptr<Node>> waypointToWaypoint;
    std::multimap<std::shared_ptr<Node>, std::shared_ptr<Node>> waypointToFinal;

    for (auto &it: runwayTransitions) {
        auto &rwy = it.first;
        auto &nodes = it.second;
        auto &last = nodes.empty() ? nullptr : nodes.back();
        runwayToWaypoint.insert(std::make_pair(rwy, last));
    }

    for (auto &it: commonRoutes) {
        auto &srcNode = it.first;
        auto &nodes = it.second;
        auto &destNode = nodes.empty() ? nullptr : nodes.back();

        if (srcNode->isRunway()) {
            auto rw = std::dynamic_pointer_cast<Runway>(srcNode);
            runwayToWaypoint.insert(std::make_pair(rw, destNode));
        } else {
            waypointToWaypoint.insert(std::make_pair(srcNode, destNode));
        }
    }

    for (auto &nodes: enrouteTransitions) {
        auto &srcNode = nodes.front();
        auto &finalFix = nodes.back();
        waypointToFinal.insert(std::make_pair(srcNode, finalFix));
    }

    auto accept = [&f] (std::shared_ptr<Runway> rw, std::shared_ptr<Node> n) {
        auto fix = std::dynamic_pointer_cast<NavFix>(n);
        if (fix && fix->isGlobal()) {
            f(rw, fix);
        }
    };

    for (auto &it: runwayToWaypoint) {
        auto &rw = it.first;
        auto &waypoint = it.second;
        auto wpIt = waypointToWaypoint.find(waypoint);
        std::shared_ptr<Node> curFix = waypoint;
        if (wpIt != waypointToWaypoint.end()) {
            curFix = wpIt->second;
        }

        auto range = waypointToFinal.equal_range(curFix);
        if (range.first == range.second) {
            accept(rw, curFix);
            continue;
        }

        for (auto finalIt = range.first; finalIt != range.second; ++finalIt) {
            accept(rw, finalIt->second);
        }
    }
}



NodeList STARModel::getWaypoints(std::shared_ptr<Runway> arrivalRwy, std::string starTransName) const
{

    std::string runwayID = arrivalRwy ? arrivalRwy->getIdent() : "";

    if ((runwayTransitions.size() + commonRoutes.size() + enrouteTransitions.size()) == 0) {
        logger::warn("STAR %s has no waypoints", getIdent().c_str());
        return NodeList();
    }

    logger::info("STAR %s:\n%s", getIdent().c_str(), toDebugString().c_str());
    logger::info("Want from fix '%s' to runway '%s'", starTransName.c_str(), runwayID.c_str());

    /* We're going to run a 3 tier (enroute, common, runway) nested loop through
     * all permutations of a full STAR route. So transform the structures into vector<vector>
     * ensuring that all outer vectors have at least an empty vector inside so as to give the
     * appropriate loop a once through. If the map key NavNode is not in the vector<NavNcde>
     * then add it.
     */
    std::vector<NodeList> ee;
    std::vector<NodeList> cc;
    std::vector<NodeList> rr;
    NodeList empty;
    if (enrouteTransitions.size() > 0) {
        ee = enrouteTransitions;
    } else {
        ee.push_back(empty);
    }
    if (commonRoutes.size() > 0) {
        for (auto it: commonRoutes) {
            auto w = it.second;
            if (std::find(w.begin(), w.end(), it.first) == w.end()) {
                w.push_back(it.first);
            }
            cc.push_back(w);
        }
    } else {
        cc.push_back(empty);
    }
    if (runwayTransitions.size() > 0) {
        for (auto it: runwayTransitions) {
            auto w = it.second;
            if (std::find(w.begin(), w.end(), it.first) == w.end()) {
                // Put runway at end of list
                w.push_back(it.first);
            }
            rr.push_back(w);
        }
    } else {
        rr.push_back(empty);
    }

    /* 3 tier loop through all permutations of (common, enroute, runway)
     * Check if a permutation meets the constraints of enroute and runway that may or may
     * not have been specified in the flight plan.
     * Maintain a list of common waypoints in each of the successive validated permutations.
     * This will often end up the same as one of the entries in the commonRoutes map.
     * But sometimes there may be a common route even if there are no entries in the commonRoute map.
     * And sometimes the common route returned can be larger than an entry in the commonRoute map.
     */
    std::vector<NodeList> routePermutations;
    NodeList repeatedWaypoints;
    logger::info("Possible routes are:");
    for (auto c: cc) {
        for (auto e: ee) {
            for (auto r: rr) {
                NodeList waypoints;
                waypoints.insert(waypoints.end(), e.begin(), e.end());
                waypoints.insert(waypoints.end(), c.begin(), c.end());
                waypoints.insert(waypoints.end(), r.begin(), r.end());
                auto newEnd = std::unique(waypoints.begin(), waypoints.end());
                waypoints.erase(newEnd, waypoints.end());

                if (!waypoints.empty() && !starTransName.empty() && (waypoints.front()->getIdent() != starTransName)) {
                    // Specified transition doesn't match - unsuitable
                    continue;
                }
                if (!waypoints.empty() && waypoints.back()->isRunway() && arrivalRwy && (waypoints.back() != arrivalRwy)) {
                    // Specified arrival runway doesn't match - unsuitable
                    continue;
                }
                if (repeatedWaypoints.empty()) {
                    // Initialise
                    repeatedWaypoints = waypoints;
                } else {
                    // Filter the list of repeated waypoints using new valid permutation
                    auto iter = std::remove_if(std::begin(repeatedWaypoints), std::end(repeatedWaypoints), [&waypoints] (const auto &a) -> bool {
                        return (find(waypoints.begin(), waypoints.end(), a) == waypoints.end());
                    });
                    repeatedWaypoints.erase(iter, std::end(repeatedWaypoints));
                }
                routePermutations.push_back(waypoints);

                std::stringstream ss;
                for (auto w: waypoints) {
                    ss << w->getIdent() << " ";
                }
                logger::info(" %s", ss.str().c_str());
            }
        }
    }

    switch(routePermutations.size()) {
    case 0:  logger::warn("No waypoints found");
             return NodeList();
    case 1:  return routePermutations.front();
    default: std::stringstream ss;
             for (auto w: repeatedWaypoints) {
                 ss << w->getIdent() << " ";
             }
             logger::info("Using common waypoints: %s", ss.str().c_str());
             return repeatedWaypoints;
    }

    return NodeList();
}

void STARModel::iterate(std::function<void(std::shared_ptr<Runway>, std::shared_ptr<NavFix>, std::shared_ptr<Node>)> f) const
{
    // going to key (!), maps show where to come from
    std::multimap<std::shared_ptr<Node>, std::shared_ptr<Node>> routeToWaypoint;
    std::map<std::shared_ptr<Node>, std::shared_ptr<Node>> waypointToWaypoint;
    std::map<std::shared_ptr<Runway>, std::shared_ptr<Node>> runwayToApproachStart;
    std::map<std::shared_ptr<Runway>, std::shared_ptr<Node>> runwayToApproachEnd;

    for (auto &nodes: enrouteTransitions) {
        auto &startFix = nodes.front();
        auto &waypoint = nodes.back();
        routeToWaypoint.insert(std::make_pair(waypoint, startFix));
    }

    for (auto &it: commonRoutes) {
        auto &keyNode = it.first;
        auto &nodes = it.second;
        auto &srcNode = nodes.empty() ? nullptr : nodes.front();
        auto &dstNode = nodes.empty() ? nullptr : nodes.back();

        if (keyNode->isRunway()) {
            auto rw = std::dynamic_pointer_cast<Runway>(keyNode);
            runwayToApproachStart.insert(std::make_pair(rw, srcNode));
            runwayToApproachEnd.insert(std::make_pair(rw, dstNode));
        }
        waypointToWaypoint.insert(std::make_pair(dstNode, srcNode));
    }

    for (auto &it: runwayTransitions) {
        auto &rwy = it.first;
        auto &nodes = it.second;
        auto &first = nodes.empty() ? nullptr : nodes.front();
        auto &last = nodes.empty() ? nullptr : nodes.back();
        runwayToApproachStart.insert(std::make_pair(rwy, first));
        runwayToApproachEnd.insert(std::make_pair(rwy, last));
    }

    auto accept = [&f] (std::shared_ptr<Runway> rw, std::shared_ptr<Node> start, std::shared_ptr<Node> end) {
        auto fix = std::dynamic_pointer_cast<NavFix>(start);
        if (fix && fix->isGlobal()) {
            f(rw, fix, end);
        }
    };

    for (auto &it: runwayToApproachStart) {
        auto &rw = it.first;
        auto &approachPoint = it.second;
        auto wpIt = waypointToWaypoint.find(approachPoint);
        std::shared_ptr<Node> curFix = approachPoint;
        if (wpIt != waypointToWaypoint.end()) {
            curFix = wpIt->second;
        }

        auto range = routeToWaypoint.equal_range(curFix);
        if (range.first == range.second) {
            accept(rw, curFix, runwayToApproachEnd[rw]);
            continue;
        }

        for (auto startIt = range.first; startIt != range.second; ++startIt) {
            accept(rw, startIt->second, runwayToApproachEnd[rw]);
        }
    }
}


NodeList ApproachModel::getWaypoints(std::shared_ptr<Runway> arrivalRwy, std::string appTransName) const
{
    if ((transitions.size() + approach.size()) == 0) {
        logger::warn("Approach %s has no waypoints", getIdent().c_str());
        return NodeList();
    }

    logger::info("Approach %s from fix '%s'\n%s",
        getIdent().c_str(), appTransName.c_str(), toDebugString().c_str());

    NodeList waypoints;
    if (transitions.empty() || appTransName.empty()) {
        waypoints = approach;
    } else {
        // Populate waypoints from requested transition then the approach waypoints
        try {
            waypoints = transitions.at(appTransName);
        } catch (std::out_of_range &e) {
            logger::warn("Couldn't find '%s' in approach transitions", appTransName.c_str());
        }
        waypoints.insert(waypoints.end(), approach.begin(), approach.end());

        auto newEnd = std::unique(waypoints.begin(), waypoints.end());
        waypoints.erase(newEnd, waypoints.end());
    }

    // Remove all waypoints after any runway as these are missed approach
    for (auto it = waypoints.begin(); it != waypoints.end(); it++) {
        if ((*it)->isRunway()) {
            waypoints.erase(it + 1, waypoints.end());
        }
    }

    std::stringstream ss;
    for (auto w: waypoints) {
        ss << w->getIdent() << " ";
    }
    logger::info("Approach waypoints: %s", ss.str().c_str());

    return waypoints;
}

std::string ApproachModel::toDebugString() const
{
    std::stringstream res;

    for (auto &it: transitions) {
        res << "  Transition " << it.first << " connects ";
        for (auto &node: it.second) {
            res << node->getIdent() << ", ";
        }
        res << "\n";
    }

    res << "  Actual approach: ";
    for (auto &node: approach) {
        res << node->getIdent() << ", ";
    }
    res << "\n";

    return res.str();
}

void ApproachModel::addTransition(const std::string& id, const NodeList& nodes)
{
    transitions.insert(std::make_pair(id, nodes));
}

void ApproachModel::addApproach(const NodeList &nodes)
{
    approach = nodes;
}

const std::shared_ptr<NavFix> ApproachModel::getStartFix() const
{
    if (approach.empty()) {
        return nullptr;
    }
    return std::dynamic_pointer_cast<NavFix>(approach.front());
}

const std::shared_ptr<Runway> ApproachModel::getRunway() const
{
    for (auto &node: approach) {
        if (node->isRunway()) {
            return std::dynamic_pointer_cast<Runway>(node);
        }
    }
    return nullptr;
}

void ApproachModel::iterateTransitions(std::function<void(const std::string&, std::shared_ptr<NavFix>, std::shared_ptr<Runway>)> f)
{
    for (auto &it: transitions) {
        if (it.second.empty()) {
            continue;
        }
        auto startFix = std::dynamic_pointer_cast<NavFix>(it.second.front());
        if (startFix && startFix->isGlobal()) {
            f(it.first, startFix, getRunway());
        }
    }
}


} /* namespace navdb */


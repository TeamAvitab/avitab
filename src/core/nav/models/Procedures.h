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
#include <functional>
#include "Navigation.h"

namespace navdb {

class ProcedureOptions
{
public:
    ProcedureOptions(std::string id);
    ProcedureOptions() = delete;

    void attachRunwayTransition(std::shared_ptr<Runway> rwy, const NodeList &nodes);
    void attachCommonRoute(std::shared_ptr<Node> start, const NodeList &nodes);
    void attachEnrouteTransitions(const NodeList &nodes);

protected:
    std::string toDebugString() const;

protected:
    std::map<std::shared_ptr<Runway>, NodeList> runwayTransitions;
    std::map<std::shared_ptr<Node>, NodeList> commonRoutes;
    std::vector<NodeList> enrouteTransitions;

private:
    std::string transitionId;

};

class SIDModel : public SID, public ProcedureOptions
{
public:
    SIDModel(const std::string &id) : ProcedureOptions(id), sidID(id) { }
    const std::string& getIdent() const override { return sidID; }
    
    NodeList getWaypoints(std::shared_ptr<Runway> departureRwy, std::string sidTransName) const override;
    std::string toDebugString() const override { return ProcedureOptions::toDebugString(); }

    void iterate(std::function<void(std::shared_ptr<Runway>, std::shared_ptr<NavFix>)> f) const;
private:
    std::string const sidID;
};

class STARModel : public STAR, public ProcedureOptions
{
public:
    STARModel(const std::string &id) : ProcedureOptions(id), starID(id) { }
    const std::string& getIdent() const override { return starID; }

    NodeList getWaypoints(std::shared_ptr<Runway> arrivalRwy, std::string starTransName) const override;
    std::string toDebugString() const override { return ProcedureOptions::toDebugString(); }

    void iterate(std::function<void(std::shared_ptr<Runway>, std::shared_ptr<NavFix>, std::shared_ptr<Node>)> f) const;
private:
    std::string const starID;
};

class ApproachModel : public Approach, public ProcedureOptions
{
public:
    ApproachModel(const std::string &id) : ProcedureOptions(id), approachID(id) { }
    const std::string& getIdent() const override { return approachID; }

    NodeList getWaypoints(std::shared_ptr<Runway> arrivalRwy, std::string appTransName) const override;
    std::string toDebugString() const override;

    void addTransition(const std::string &id, const NodeList &nodes);
    void addApproach(const NodeList &nodes);
    void iterateTransitions(std::function<void(const std::string &, std::shared_ptr<NavFix>, std::shared_ptr<Runway>)> f);

    const std::shared_ptr<NavFix> getStartFix() const;

private:
    const std::shared_ptr<Runway> getRunway() const;

private:
    std::string const approachID;
    std::map<std::string, NodeList> transitions;
    NodeList approach;

};



}
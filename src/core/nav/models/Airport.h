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
#include <map>
#include <vector>
#include <set>
#include <functional>
#include "Navigation.h"
#include "WorldGeometry.h"

namespace navdb {

class RadioFrequency;
class RunwayModel;
class HeliportModel;
class NavFixModel;
class SIDModel;
class STARModel;
class ApproachModel;

class AirportModel : public Airport
{
public:
    AirportModel(const std::string &id) : ident(id), displayID(id) { }

    const std::string &getIdent() const override { return ident; }
    const std::string &getName() const override { return name; }
    const world::Location& getLocation() const override;
    std::shared_ptr<Region> getRegion() const override { return region; }
    int getElevationFt() const override { return elevation; }
    const world::Location &getCornerSW() const override;
    const world::Location &getCornerNE() const override;
    const std::string& getICAOCode() const override { return icaoCode; }
    const std::string& getFAACode() const override { return faaCode; }
    const std::string& getLocalCode() const override { return localCode; }
    const std::string& getDisplayID() const override { return displayID; }
    const std::vector<RadioFrequency> &getATCFrequencies(ATCserviceType type) override { return atcFrequencies[type]; }
    bool hasHeliports() const override;
    bool hasOnlyHeliports() const override;
    bool hasOnlyWaterRunways() const override;
    bool hasHardRunway() const override;
    bool hasControlTower() const override;
    bool hasATCFrequencies() const override;
    float getLongestRunwayLength() const override;
    const std::shared_ptr<Runway> getRunwayByName(const std::string &rw) const override;
    const std::shared_ptr<Runway> getOppositeRunwayEnd(const std::shared_ptr<Runway> rw) const override;
    std::shared_ptr<NavFix> getTerminalFix(const std::string &id) override;
    std::vector<std::shared_ptr<SID>> getSIDs() const override;
    std::vector<std::shared_ptr<STAR>> getSTARs() const override;
    std::vector<std::shared_ptr<Approach>> getApproaches() const override;
    std::shared_ptr<SID> getSIDByName(std::string sidName) const override;
    std::shared_ptr<STAR> getSTARByName(std::string starName) const override;
    std::shared_ptr<Approach> getApproachByName(std::string appName) const override;
    std::string getInitialATCContactInfo() const override;
    void forEachRunway(std::function<void(const std::shared_ptr<Runway>)> f) const override;
    void forEachRunwayPair(std::function<void(const std::shared_ptr<Runway>, const std::shared_ptr<Runway>)> f) const override;
    void forEachHeliport(std::function<void(const std::shared_ptr<Heliport>)> f) const override;

    void setIdent(const std::string &id) { ident = id; }
    void setName(const std::string &n) { name = n; }
    void setLocation(const world::Location &loc);
    void setRegion(std::shared_ptr<Region> r) { region = r; }
    void setElevation(int e) { elevation = e; }
    void setICAOCode(const std::string &icaoCode);
    void setFAACode(const std::string &faaCode);
    void setLocalCode(const std::string &localCode);
    void addATCFrequency(ATCserviceType which, const RadioFrequency &frq) { atcFrequencies[which].push_back(frq); }
    void addRunway(std::shared_ptr<RunwayModel> rwy);
    void addRunwayEnds(std::shared_ptr<RunwayModel> rwy1, std::shared_ptr<RunwayModel> rwy2);
    void addHeliport(std::shared_ptr<HeliportModel> port);
    void addTerminalFix(std::shared_ptr<NavFixModel> fix);
    void attachILSData(const std::string& rwyName, std::shared_ptr<NavFix> fix);

    void addSID(std::shared_ptr<SIDModel> sid);
    void addSTAR(std::shared_ptr<STARModel> star);
    void addApproach(std::shared_ptr<ApproachModel> approach);

private:
    std::shared_ptr<RunwayModel> getClosestRunway(const std::string &name);

private:
    std::string ident; // either ICAO code or X + fictional id
    std::string displayID; // either Xplane ID or ICAO, FAA or Local code
    std::string name;
    world::Location location;
    world::Location locSW;
    world::Location locNE;
    int elevation = 0; // feet AMSL

    // Optional
    std::string faaCode;
    std::string icaoCode;
    std::string localCode;
    std::shared_ptr<Region> region;
    std::map<ATCserviceType, std::vector<RadioFrequency>> atcFrequencies;

    std::map<std::string, std::shared_ptr<RunwayModel>> runways;
    std::map<std::shared_ptr<RunwayModel>, std::shared_ptr<RunwayModel>> runwayPairs;
    std::map<std::string, std::shared_ptr<HeliportModel>> heliports;

    std::map<std::string, std::shared_ptr<NavFixModel>> terminalFixes;
    std::map<std::string, std::shared_ptr<SIDModel>> sids;
    std::map<std::string, std::shared_ptr<STARModel>> stars;
    std::map<std::string, std::shared_ptr<ApproachModel>> approaches;
};

} /* namespace navdb */

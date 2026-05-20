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
#include "Navigation.h"
#include "WorldGeometry.h"

namespace navdb {

struct RadioNavaidModel
{
    RadioNavaidModel(const RadioFrequency &rf, int rng) : frequency(rf), range(rng) { }
    RadioFrequency const frequency;
    int const range;
};

class NDBModel : public NDB
{
public:
    NDBModel(RadioFrequency frq, int range) : navaid(frq, range) { }
    const RadioFrequency &getFrequency() const override { return navaid.frequency; }
    int getRange() const override { return navaid.range; }
private:
    RadioNavaidModel navaid;
};

class VORModel : public VOR
{
public:
    VORModel(RadioFrequency frq, int range, double brng) :
        navaid(frq, range), bearing(brng) { }
    const RadioFrequency &getFrequency() const override { return navaid.frequency; }
    int getRange() const override { return navaid.range; }
    float getBearing() const override { return bearing; }
private:
    RadioNavaidModel navaid;
    double bearing;
};

class DMEModel : public DME
{
public:
    DMEModel(RadioFrequency frq, int range, bool p) : navaid(frq, range), paired(p) { }
    const RadioFrequency &getFrequency() const override { return navaid.frequency; }
    int getRange() const override { return navaid.range; }
    bool isPaired() const override { return paired; }
private:
    RadioNavaidModel navaid;
    bool paired = false;
};

class ILSLocalizerModel : public ILSLocalizer
{
public:
    ILSLocalizerModel(RadioFrequency frq, int range, double heading, double magbearing, bool isOnlyLocalizer) :
        navaid(frq, range), runwayHeading(heading), runwayHeadingMagnetic(magbearing), localizerOnly(isOnlyLocalizer) { }
    const RadioFrequency &getFrequency() const override { return navaid.frequency; }
    int getRange() const override { return navaid.range; }
    double getRunwayHeading() const override { return runwayHeading; }
    double getRunwayHeadingMagnetic() const override { return runwayHeadingMagnetic; }
    bool isLocalizerOnly() const override { return localizerOnly; }
private:
    RadioNavaidModel navaid;
    double runwayHeading = 0;
    double runwayHeadingMagnetic = 0;
    bool localizerOnly = false;
};

class NavFixModel : public NavFix
{
public:
    NavFixModel(std::shared_ptr<Region> r, const std::string id, world::Location loc) : region(r), ident(id), location(loc) { }
    const std::string& getIdent() const override { return ident; }
    const world::Location& getLocation() const override { return location; }
    bool isGlobal() const override { return global; }
    bool hasNavaid() const override { return navaid; }
    std::shared_ptr<Region> getRegion() const override { return region; }
    void setGlobal() { global = true; }
    const NDB *getNDB() const override { return ndb.get(); }
    const VOR *getVOR() const override { return vor.get(); }
    const DME *getDME() const override { return dme.get(); }
    const ILSLocalizer *getILSLocalizer() const override { return ilsLoc.get(); }

    void attachNDB(std::unique_ptr<NDBModel> &ptr) { ndb = std::move(ptr); navaid = true; }
    void attachVOR(std::unique_ptr<VORModel> &ptr) { vor = std::move(ptr); navaid = true; }
    void attachDME(std::unique_ptr<DMEModel> &ptr) { dme = std::move(ptr); navaid = true; }
    void attachILSLocalizer(std::unique_ptr<ILSLocalizerModel> &ptr) { ilsLoc = std::move(ptr); navaid = true; }

private:
    std::shared_ptr<Region> const region;
    std::string const ident;
    world::Location const location;
    bool global { false };
    bool navaid { false };

    // Optional
    std::unique_ptr<NDBModel> ndb;
    std::unique_ptr<VORModel> vor;
    std::unique_ptr<DMEModel> dme;
    std::unique_ptr<ILSLocalizerModel> ilsLoc;
};

} /* namespace navdb */

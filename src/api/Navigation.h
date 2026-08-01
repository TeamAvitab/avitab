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

#include <map>
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <filesystem>

namespace world {
class Location;
}

namespace navdb {

// Abstract interfaces to all of the navigation database entities. There are lots of them, but
// combined in this one flat file for simplicity.

enum class AirwayLevel {
    UPPER,
    LOWER
};

enum class FrequencyUnit {
    NONE,
    MHZ,
    KHZ
};

enum class SurfaceMaterial {
    ASPHALT,
    BITUMINOUS,
    CONCRETE,
    CORAL,
    DIRT,
    GRASS,
    GRAVEL,
    ICE,
    LAKEBED,
    SAND,
    SNOW,
    TARMAC,
    WATER,
    CUSTOM,
    UNKNOWN
};

enum class ATCserviceType { // was ATCFrequency
    RECORDED,   // ATIS, AWOS, ASOS
    UNICOM,     // also CTAF
    MULTICOM,
    FSS,
    CLD,        // also CPT
    GND,
    TWR,
    APP,
    DEP,
    CTR
};

enum class UserFixType {
    NONE,
    VRP,
    POI,
    MARKER,
    ROUTEFIX
};


class NamedNavItem {
public:
    virtual const std::string& getIdent() const = 0;
    virtual const std::string& getName() const { return getIdent(); } // subclasses can override if name is different from identity
    virtual ~NamedNavItem() = default;
};

using Region = NamedNavItem;

constexpr const char *UserRegionIdent = "USER"; // used for user fixes and routes

class Node : public NamedNavItem {
public:
    virtual const world::Location& getLocation() const = 0;
    virtual bool isAirport() const { return false; }
    virtual bool isFix() const { return false; }
    virtual bool isRunway() const  { return false; }
};

using NodeList = std::vector<std::shared_ptr<Node>>;

class NodeLink : public NamedNavItem {
public:
    virtual bool supportsLevel(AirwayLevel level) const { return true; }
    virtual bool isProcedure() const { return false; }
};

class RadioFrequency
{
public:
    RadioFrequency(int frq, int places, FrequencyUnit unit, const std::string &desc);
    const std::string &getDescription() const { return description; }
    std::string getFrequencyString(bool appendUnits = true) const;
private:
    int frequency = 0;
    int places = 0;
    FrequencyUnit unit = FrequencyUnit::NONE;
    std::string description;
};

class RadioNavaid {
public:
    virtual const RadioFrequency &getFrequency() const = 0;
    virtual int getRange() const = 0;

    virtual ~RadioNavaid() = default;
};

class NDB : public RadioNavaid {
public:
};

class VOR : public RadioNavaid {
public:
    virtual float getBearing() const = 0;
};

class DME : public RadioNavaid {
public:
    virtual bool isPaired() const = 0;
};

class ILSLocalizer : public RadioNavaid {
public:
    virtual double getRunwayHeading() const = 0;
    virtual double getRunwayHeadingMagnetic() const = 0;
    virtual bool isLocalizerOnly() const = 0;
};

class Fix : public Node {
public:
    bool isFix() const override { return true; }

    virtual bool isGlobal() const = 0;
    virtual bool hasNavaid() const { return false; }
    virtual bool isUserFix() const { return false; }
    virtual std::shared_ptr<Region> getRegion() const = 0;
};

class NavFix : public Fix {
public:
    virtual const NDB *getNDB() const = 0;
    virtual const VOR *getVOR() const = 0;
    virtual const DME *getDME() const = 0;
    virtual const ILSLocalizer *getILSLocalizer() const = 0;
};

class UserFix : public Fix {
public:
    bool isGlobal() const override { return false; }
    bool isUserFix() const override { return true; }

    virtual UserFixType getType() const = 0;
};

class Runway: public Node {
public:
    bool isRunway() const override { return true; }

    virtual float getHeading() const = 0;
    virtual float getLength() const = 0;
    virtual float getWidth() const = 0;
    virtual float getElevation() const = 0;
    virtual bool hasHardSurface() const = 0;
    virtual bool isWater() const = 0;
    virtual SurfaceMaterial getSurfaceType() const = 0;
    virtual const char *getSurfaceTypeDescription() const = 0;
    virtual std::shared_ptr<NavFix> getNavaidILS() const = 0;
};

class Heliport: public Node {
public:
    bool isAirport() const override { return true; }

    virtual float getLength() const = 0;
    virtual float getWidth() const = 0;
};

class Airway: public NodeLink {
public:
    virtual AirwayLevel getLevel() const = 0;
};

class Procedure : public NodeLink {
public:
    bool isProcedure() const override { return true; }
    virtual NodeList getWaypoints(std::shared_ptr<Runway> runway, std::string appTransName) const = 0;
    virtual std::string toDebugString() const = 0;
};

using Approach = Procedure;
using SID = Procedure;
using STAR = Procedure;

class Airport: public Node {
public:
    bool isAirport() const override { return true; }

    virtual std::shared_ptr<Region> getRegion() const = 0;
    virtual const world::Location &getCornerSW() const = 0;
    virtual const world::Location &getCornerNE() const = 0;

    virtual int getElevationFt() const = 0;

    virtual const std::string& getICAOCode() const = 0;
    virtual const std::string& getFAACode() const = 0;
    virtual const std::string& getLocalCode() const = 0;
    virtual const std::string& getDisplayID() const = 0;

    virtual const std::vector<RadioFrequency> &getATCFrequencies(ATCserviceType type) = 0;

    virtual bool hasHeliports() const = 0;
    virtual bool hasOnlyHeliports() const = 0;
    virtual bool hasOnlyWaterRunways() const = 0;
    virtual bool hasHardRunway() const = 0;
    virtual bool hasControlTower() const = 0;
    virtual bool hasATCFrequencies() const = 0;

    virtual float getLongestRunwayLength() const = 0;
    virtual const std::shared_ptr<Runway> getRunwayByName(const std::string &rw) const = 0;
    virtual const std::shared_ptr<Runway> getOppositeRunwayEnd(const std::shared_ptr<Runway> rw) const = 0;

    virtual std::shared_ptr<NavFix> getTerminalFix(const std::string &id) = 0;
    virtual std::vector<std::shared_ptr<SID>> getSIDs() const = 0;
    virtual std::vector<std::shared_ptr<STAR>> getSTARs() const = 0;
    virtual std::vector<std::shared_ptr<Approach>> getApproaches() const = 0;
    virtual std::shared_ptr<SID> getSIDByName(std::string sidName) const = 0;
    virtual std::shared_ptr<STAR> getSTARByName(std::string starName) const = 0;
    virtual std::shared_ptr<Approach> getApproachByName(std::string appName) const = 0;
    virtual std::string getInitialATCContactInfo() const = 0;

    virtual void forEachRunway(std::function<void(const std::shared_ptr<Runway>)> f) const = 0;
    virtual void forEachRunwayPair(std::function<void(const std::shared_ptr<Runway>, const std::shared_ptr<Runway>)> f) const = 0;
    virtual void forEachHeliport(std::function<void(const std::shared_ptr<Heliport>)> f) const = 0;
};

// The interface to an object that implements a database of all the navigation entities,
// mainly providing functions for searching the database.

class NavDatabase {
public:
    static constexpr const int MAX_SEARCH_RESULTS = 50;
    static constexpr const int MAX_DISPLAY_RESULTS = 10;

    static constexpr const int VISIT_TOWERED_AIRPORTS = 0b1;
    static constexpr const int VISIT_OTHER_AIRPORTS =   0b10;
    static constexpr const int VISIT_FIXES =            0b100;
    static constexpr const int VISIT_NAVAIDS =          0b1000;
    static constexpr const int VISIT_USER_FIXES =       0b10000;
    static constexpr const int VISIT_EVERYTHING = (VISIT_TOWERED_AIRPORTS | VISIT_OTHER_AIRPORTS | VISIT_NAVAIDS | VISIT_FIXES | VISIT_USER_FIXES);

    using NodeAcceptor = std::function<void(const Node *)>; // REFACTOR - this needs changing to a shared_ptr before SQL NAV data is introduced.
    using Connection = std::pair<std::shared_ptr<NodeLink>, std::shared_ptr<Node>>;

    virtual int maxDensity(const world::Location &bottomLeft, const world::Location &topRight) = 0;
    virtual void visitNodes(const world::Location &bottomLeft, const world::Location &topRight, NodeAcceptor calllback, int filter) = 0;

    virtual std::shared_ptr<Airport> findAirportByID(const std::string &id) const = 0;
    virtual std::shared_ptr<Airport> findAirportByCode(const std::string &id) const = 0;
    virtual std::shared_ptr<NavFix> findFixByRegionAndID(const std::string &region, const std::string &id) const = 0;
    virtual std::vector<std::shared_ptr<Airport>> findAirport(const std::string &keyWord) const = 0;
    virtual std::vector<std::shared_ptr<NavFix>> findFix(const std::string &id) const = 0;

    virtual std::vector<Connection> &getConnections(std::shared_ptr<Node> from) = 0;
    virtual bool areConnected(std::shared_ptr<Node> from, const std::shared_ptr<Node> to) = 0;

    virtual std::shared_ptr<Region> getRegion(const std::string &code) = 0;

    // user fixes can be replaced at any time from the UI, so the API is here
    virtual void clearUserFixes() = 0;
    virtual void addUserFix(std::shared_ptr<UserFix> uf) = 0;

    // used to facilitate hot-swapping the navigation database
    enum class NavStatus { ACTIVE, RELOADING, SUPERSEDED };
    virtual NavStatus status() = 0;
    virtual std::shared_ptr<NavDatabase> latest() = 0;
};


} /* namespace navdb */

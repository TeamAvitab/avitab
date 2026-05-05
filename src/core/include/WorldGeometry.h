/* This file is part of the Avitab project. See the README and LICENSE for details. */

#pragma once

#include <cmath>
#include <limits>
#include <algorithm>
#include <utility>

// This header file defines structures and classes related to
// locations, trajectories and spherical geometry.
// The members of these classes are stored as radians since this
// matches the expectations of the cmath library and reduces the
// number of conversions that will be required.

namespace world {

// Some conversion multipliers and other useful constants
constexpr const double DEG_TO_RAD = M_PI / 180.0f;
constexpr const double RAD_TO_DEG = 180.0f / M_PI;
constexpr const double M_TO_FT = 3.28084;
constexpr const double FT_TO_M = 1 / 3.28084;
constexpr const double LAT_TO_KM = 111.133f;
constexpr const double KM_TO_NM = 0.539957;
constexpr const double MS_TO_KT = 1.94384;
constexpr const double NM_TO_RAD = M_PI * 2 / 21600;
constexpr const double KM_TO_RAD = KM_TO_NM * M_PI * 2 / 21600;
constexpr const double ESPG3857_CONST = 20037508.34;

// Locations represent a point on the globe surface. Internal representation is
// in radians, intended to reduce the number of conversions when doing maths stuff.
// But since most of the Avitab UI is in degrees, some overhead is inevitable.
struct Location
{
    double ypos_rad;                // -PI/2 .. +PI/2
    double xpos_rad;                // -PI .. +PI

    Location() : ypos_rad(std::numeric_limits<double>::quiet_NaN()), xpos_rad(std::numeric_limits<double>::quiet_NaN()) { }
    Location(const Location&) = default;
    Location(double lat, double lon);
    Location& operator=(const Location&);

    bool operator==(Location const & rhs) const { return (ypos_rad == rhs.ypos_rad) && (xpos_rad == rhs.xpos_rad); }

    virtual bool isValid() const { return !std::isnan(ypos_rad) && !std::isnan(xpos_rad); }

    double lat() const { return ypos_rad; }
    double lon() const { return xpos_rad; }
    double latDegrees() const { return ypos_rad * RAD_TO_DEG; }
    double lonDegrees() const { return xpos_rad * RAD_TO_DEG; }

    // From/to Geographic Coordinate System (ie common latitude/longitude in degrees)
    static Location fromGCS(double lat, double lon);
    std::pair<double, double> toGCS() const;            // lat, lon

    // From/to Tile YX
    static Location fromTileYX(double y, double x, int zoom);
    std::pair<double, double> toTileYX(int zoom) const; // y,x

    // From/to EPSG3857
    static Location fromEPSG3857(double y, double x);
    std::pair<double, double> toEPSG3857() const;       // y,x

    // From/to Mercator (used by calibration)
    static Location fromMercator(double y, double x);
    std::pair<double, double> toMercator() const;       // y,x

    // Get locations related to an area formed by this and another
    Location areaSouthWest(Location const & other) const;
    Location areaNorthEast(Location const & other) const;
    Location areaCenter(Location const & other) const;

    // These more complex implementations are not inlined
    double angularDistanceTo(const Location& loc) const; // always radians
    double surfaceDistanceTo(const Location& loc) const; // always metres
    bool isContainedWithin(const Location& sw, const Location& ne) const;
};

inline Location::Location(double lat, double lon)
:   ypos_rad(lat), xpos_rad(lon)
{
    if (ypos_rad > M_PI / 2) ypos_rad = M_PI / 2;
    if (ypos_rad < -(M_PI / 2)) ypos_rad = -(M_PI / 2);
    while (xpos_rad >= M_PI) xpos_rad -= 2 * M_PI;
    while (xpos_rad < -M_PI) xpos_rad += 2 * M_PI;
}

inline Location& Location::operator=(const Location& l)
{
    ypos_rad = l.ypos_rad;
    xpos_rad = l.xpos_rad;
    return *this;
}

inline Location Location::fromGCS(double lat, double lon) {
    return Location(lat * DEG_TO_RAD, lon * DEG_TO_RAD);
}

inline std::pair<double, double> Location::toGCS() const {
    return std::pair<double, double>(latDegrees(), lonDegrees());
}

inline Location Location::fromTileYX(double y, double x, int zoom)
{
    double zp = std::pow(2.0, zoom);
    double plainLon = x / zp * 360.0 - 180;
    double lon = std::fmod(plainLon, 360.0);
    if (lon > 180.0) {
        lon -= 360.0;
    } else if (lon <= -180.0) {
        lon += 360.0;
    }

    double n = M_PI - (2.0 * M_PI * y / zp);
    double lat = RAD_TO_DEG * std::atan(0.5 * (std::exp(n) - std::exp(-n)));

    return world::Location::fromGCS(lat, lon);
}

inline std::pair<double, double> Location::toTileYX(int zoom) const
{
    double zp = std::pow(2.0, zoom);
    double x = (lonDegrees() + 180.0) / 360.0 * zp;
    double y = (1.0 - std::log(std::tan(ypos_rad) + (1.0 / std::cos(ypos_rad))) / M_PI) / 2.0 * zp;
    return std::pair<double, double>(y, x);
}

inline Location Location::fromEPSG3857(double y, double x) {
    double xrad = (x / ESPG3857_CONST) * M_PI;
    double yrad = std::atan(std::exp((y / ESPG3857_CONST) * M_PI)) * 2 - (90 * DEG_TO_RAD);
    return Location(yrad, xrad);
}

inline std::pair<double, double> Location::toEPSG3857() const
{
    constexpr const double ESPG3857_SCALE = ESPG3857_CONST / M_PI;
    double lon3857 = ESPG3857_SCALE * xpos_rad;
    double lat3857 = ESPG3857_SCALE * std::log(std::tan((M_PI / 4) + (ypos_rad / 2)));
    return std::pair<double, double>(lat3857, lon3857);
}

inline Location Location::fromMercator(double latm, double lonm) {
    double yrad = std::atan(std::sinh(latm * DEG_TO_RAD));
    double xrad = lonm * DEG_TO_RAD;
    return Location(yrad, xrad);
}

inline std::pair<double, double> Location::toMercator() const
{
    double sinPhi = std::sin(ypos_rad);
    double mercLat = 0.5 * std::log((1 + sinPhi) / (1 - sinPhi)) * RAD_TO_DEG;
    return std::pair<double, double>(mercLat, lonDegrees());
}

inline Location Location::areaSouthWest(Location const & other) const {
    if (!isValid()) return other;
    if (!other.isValid()) return *this;
    return Location(std::min(ypos_rad, other.ypos_rad), std::min(xpos_rad, other.xpos_rad));
}

inline Location Location::areaNorthEast(Location const & other) const {
    if (!isValid()) return other;
    if (!other.isValid()) return *this;
    return Location(std::max(ypos_rad, other.ypos_rad), std::max(xpos_rad, other.xpos_rad));
}

inline Location Location::areaCenter(Location const & other) const {
    if (!isValid()) return other;
    if (!other.isValid()) return *this;
    return Location(((ypos_rad + other.ypos_rad) / 2), ((xpos_rad + other.xpos_rad) / 2));
}


// A Trajectory is the combination of a Location with a direction of travel
struct Trajectory : public Location
{
    double hdg_rad;                 // -PI .. +PI

    Trajectory() : Location(), hdg_rad(std::numeric_limits<double>::quiet_NaN()) { }
    Trajectory(const Trajectory&) = default;
    Trajectory(const Location& here, double heading);
    Trajectory(const Location& here, const Location &dest);
    Trajectory& operator=(const Trajectory&);

    virtual bool isValid() const { return Location::isValid() && !std::isnan(hdg_rad); }

    // Return the waypoint along the current great circle trajectory after
    // travelling an angular distance across the globe.
    Trajectory greatCircleWaypoint(double angle) const;
    double hdgDegrees() const { return (hdg_rad >= 0 ? hdg_rad : (2 * M_PI + hdg_rad)) * RAD_TO_DEG; }

    // From Geographic Coordinate System (ie common latitude/longitude and heading in degrees)
    static Trajectory fromGCS(double lat, double lon, double heading);

};

inline Trajectory::Trajectory(const Location& ref, double head)
:   Location(ref), hdg_rad(head)
{
    while (hdg_rad >= M_PI) hdg_rad -= 2 * M_PI;
    while (hdg_rad < -M_PI) hdg_rad += 2 * M_PI;
}

inline Trajectory& Trajectory::operator=(const Trajectory& t)
{
    Location::operator=(t);
    hdg_rad = t.hdg_rad;
    return *this;
}

inline Trajectory Trajectory::fromGCS(double lat, double lon, double heading)
{
    return Trajectory(Location::fromGCS(lat, lon), heading  * DEG_TO_RAD);
}

// A Position is a location, heading and altitude
struct Position : public Trajectory
{
    double alt_metres;              // metres above sea-level

    Position() : Trajectory(), alt_metres(std::numeric_limits<double>::quiet_NaN()) { }
    Position(const Position&) = default;
    Position(const Trajectory &t, double alt) : Trajectory(t), alt_metres(alt) { }
    Position& operator=(const Position&);
    Position& operator=(const Trajectory&);

    virtual bool isValid() const { return Trajectory::isValid() && !std::isnan(alt_metres); }

    double altFeet() const { return alt_metres * M_TO_FT; }

    // From Geographic Coordinate System (ie common latitude/longitude and heading in degrees)
    static Position fromGCSm(double lat, double lon, double heading, double alt_m);
    static Position fromGCSft(double lat, double lon, double heading, double alt_ft);
};

inline Position& Position::operator=(const Trajectory& t)
{
    Trajectory::operator=(t);
    return *this;
}

inline Position& Position::operator=(const Position& p)
{
    Trajectory::operator=(p);
    alt_metres = p.alt_metres;
    return *this;
}

inline Position Position::fromGCSm(double lat, double lon, double heading, double alt_m)
{
    return Position(Trajectory::fromGCS(lat, lon, heading), alt_m);
}

inline Position Position::fromGCSft(double lat, double lon, double heading, double alt_ft)
{
    return Position(Trajectory::fromGCS(lat, lon, heading), alt_ft * FT_TO_M);
}

void RunGeometryUnitTests();

} // namespace world

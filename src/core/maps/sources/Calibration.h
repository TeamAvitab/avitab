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

#include <utility>
#include <string>
#include "libimg/stitcher/TileSource.h"
#include "LinearEquation.h"
#include <nlohmann/json.hpp>

namespace maps {

class Calibration {
public:
    void setPoint1(double x, double y, world::Location loc);
    void setPoint2(double x, double y, world::Location loc);
    void setPoint3(double x, double y, world::Location loc);
    void setAngle(double angle);
    void setPreRotate(int angle);
    void setHash(const std::string &s);

    std::string toString()const;
    void fromJsonString(const std::string &s, double aspectRatio);
    void fromKmlString(const std::string &s);
    void fromLocalJson(const nlohmann::json &json);
    void fromChartfoxJson(const nlohmann::json &json, double aspectRatio);

    bool hasCalibration() const;

    img::Point<double> worldToPixels(const world::Location &loc) const;
    world::Location pixelsToWorld(double x, double y) const;
    int getPreRotate() const;
    double getNorthOffset() const;
    std::string getReport() const;

private:
    int preRotate{};
    double regX1{}, regY1{};
    double regX2{}, regY2{};
    double regX3{}, regY3{};
    world::Location regLoc1, regLoc2, regLoc3;
    std::string regHash{};
    double northOffsetAngle{};
    bool isCalibrated = false;
    bool isChartfoxGeoreferenced = false;
    bool offsetAngleDefined = false;
    std::string report{"No calibration"};
    bool dbg = false;

    LinearEquation leWorldToPixels;
    LinearEquation lePixelsToWorld;

    void calculateLocalCalibration();
    void calculateChartfoxCalibration(double k, double transformAngle, double tx, double ty, double aspectRatio);
    std::pair<double, double> rotate(double x, double y, double angleDegrees) const;
    std::string getKmlTagData(const std::string kml, const std::string tag) const;
    void define2PAThirdVertex();
    double getTriangleInnerAngleA(double a, double b, double c) const;
    double getTriangleSmallestInnerAngle() const;
    double getTriangleArea() const;
    void logDebugInfo() const;
    bool checkNoNanCoefficients();
    bool checkRefRecalculation();
    bool checkCornerRoundTrip();
    bool checkTriangle();

    static constexpr double MAXIMUM_REF_RECALCULATION_ERROR = 0.01;
    static constexpr double RECOMMENDED_REF_RECALCULATION_ERROR = 0.001;
    static constexpr double BEST_REF_RECALCULATION_ERROR = 0.0;
    static constexpr double MINIMUM_SMALLEST_ANGLE = 0.1;
    static constexpr double RECOMMENDED_SMALLEST_ANGLE = 1;
    static constexpr double BEST_SMALLEST_ANGLE = 60;
    static constexpr double MINIMUM_AREA = 0.0002;
    static constexpr double RECOMMENDED_AREA = 0.002;
    static constexpr double BEST_AREA = 0.5;
};

} /* namespace maps */

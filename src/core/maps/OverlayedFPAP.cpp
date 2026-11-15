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

#include "OverlayedFPAP.h"
#include <cmath>
#include <tuple>
#include <sstream>
#include <iomanip>


namespace maps {

OverlayedFPAP::OverlayedFPAP(IOverlayHelper *h, const navdb::NavFix *f)
:   OverlayedFix(h, f),
    navFPAP(f->getFPAP())
{
}

void OverlayedFPAP::configure(const OverlayConfig &cfg, const world::Location &loc)
{
    OverlayedFix::configure(cfg, loc);
    enabled = navFPAP && cfg.drawFPAPs && cfg.drawAirports;
    if (enabled) {
        // the FPAP is beyond the far end of the runway, the final approach starts at the threshold
        startX = posX;
        startY = posY;
        if (auto threshold = navFPAP->getThreshold()) {
            std::tie(startY, startX) = overlayHelper->locationToPixel(*threshold);
        }
        setTailCoords();
        textLocationX =  (startX + 4 * cx) / 5;       // Draw NavTextBox 1/5 of the way from end of tail to airport
        textLocationY = ((startY + 4 * cy) / 5) - 20; // And up a bit
    }
}

void OverlayedFPAP::drawGraphic()
{
    if (!enabled) return;

    auto mapImage = overlayHelper->getMapImage();
    mapImage->drawLineAA(startX, startY, lx, ly, color);
    mapImage->drawLineAA(startX, startY, cx, cy, color);
    mapImage->drawLineAA(startX, startY, rx, ry, color);
    mapImage->drawLineAA(lx, ly, rx, ry, color);
}

void OverlayedFPAP::drawText(bool detailed)
{
    if (!enabled) return;

    std::string type = navFPAP->getApproachType().empty() ? "RNP" : "RNP/" + navFPAP->getApproachType();
    if (detailed) {
        std::string channel = "";
        std::string id = "   " + getID() + "   ";
        double course = navFPAP->getApproachCourse();
        std::ostringstream str;
        if (!std::isnan(course)) {
            // the data only provides the true course
            str << std::setfill('0') << std::setw(3) << (int(std::floor(course + 0.5)) % 360) << "T";
        }
        // an FPAP transmits no ident, so no morse code
        drawNavTextBox(type, id, channel, textLocationX, textLocationY,
                       color, str.str(), false);
    } else {
        auto mapImage = overlayHelper->getMapImage();
        mapImage->drawText(getID(), 12, textLocationX, textLocationY,
                           color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    }
}

OverlayedNode::Hotspot OverlayedFPAP::getClickHotspot() const {
    return Hotspot(textLocationX, textLocationY);
}

void OverlayedFPAP::polarToCartesian(float radius, float angleDegrees, double& x, double& y) {
    x =  std::sin(angleDegrees * world::DEG_TO_RAD) * radius;
    y = -std::cos(angleDegrees * world::DEG_TO_RAD) * radius; // 0 degrees is up, decreasing y values
}

void OverlayedFPAP::setTailCoords()
{
    // the feather points against the approach course
    double tailHeading = std::fmod(navFPAP->getApproachCourse() + 180.0, 360);
    tailHeading += overlayHelper->getNorthOffset();
    double rangePixels = 0.0;
    auto mw = overlayHelper->getMapWidthNM();
    if (mw != 0) {
        auto mapImage = overlayHelper->getMapImage();
        rangePixels = (FEATHER_LENGTH_NM * mapImage->getWidth()) / mw;
    }
    double dcx, dcy, dlx, dly, drx, dry;
    const double OUTER_ANGLE = 2.5;
    polarToCartesian(rangePixels, tailHeading - OUTER_ANGLE, dlx, dly);
    polarToCartesian(rangePixels * std::cos(OUTER_ANGLE * world::DEG_TO_RAD), tailHeading, dcx, dcy);
    polarToCartesian(rangePixels, tailHeading + OUTER_ANGLE, drx, dry);

    lx = startX + dlx;
    ly = startY + dly;
    cx = startX + dcx;
    cy = startY + dcy;
    rx = startX + drx;
    ry = startY + dry;
}

} /* namespace maps */

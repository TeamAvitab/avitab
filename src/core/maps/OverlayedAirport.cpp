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
#include <cmath>
#include <cassert>
#include "OverlayedAirport.h"
#include "World.h"

namespace maps {

OverlayedAirport::OverlayedAirport(IOverlayHelper *h, const world::Airport *a):
    OverlayedNode(h, true),
    airport(a)
{
    type = getAerodromeType();
    color = getAirportColor();
}

std::string OverlayedAirport::getID() const {
    return airport->getID();
}

void OverlayedAirport::drawGraphic() {
    if (isBlob()) {
        drawAirportBlob();
    } else if (overlayHelper->getMapWidthNM() < DRAW_GEOGRAPHIC_RUNWAYS_AT_MAPWIDTHNM) {
        if (type == AerodromeType::HELIPORT) {
            drawAirportICAORing();
        } else {
            drawAirportGeographicRunways();
        }
    } else {
        if ((type == AerodromeType::HELIPORT) || (type == AerodromeType::SEAPORT) || (type == AerodromeType::AIRSTRIP)) {
            drawAirportICAORing();
        } else {
            int maxDistance = getRunwaysDrawRadius();
            if (maxDistance > ICAO_CIRCLE_RADIUS) {
                drawAirportICAOGeographicRunways();
            } else {
                drawAirportICAOCircleAndRwyPattern();
            }
        }
    }
}

void OverlayedAirport::drawText(bool detailed) {
    // Allow detailed text to be rendered for hotspots, even if just blob
    if (isBlob() && !detailed) {
        return;
    }
    // Place text below southern airport boundary and below symbol
    auto &sw = airport->getCornerSW();
    assert(sw.isValid());
    auto pixYX = overlayHelper->locationToPixel(sw);
    auto &yOffset = pixYX.first;
    yOffset += ICAO_CIRCLE_RADIUS;

    auto mapImage = overlayHelper->getMapImage();

    if (detailed) {
        std::string nameAndID = airport->getName() + " (" + airport->getDisplayID() + ")";
        std::string elevationFeet = std::to_string(airport->getElevation());
        int rwyLengthHundredsFeet = (airport->getLongestRunwayLength() * world::M_TO_FT) / 100.0;
        std::string rwyLength = (rwyLengthHundredsFeet == 0) ? "" : (" " + std::to_string(rwyLengthHundredsFeet));
        std::string atcInfo = airport->getInitialATCContactInfo();
        std::string airportInfo = " " + elevationFeet + rwyLength + " " + atcInfo + " ";
        mapImage->drawText(nameAndID,   14, posX, yOffset,      color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
        mapImage->drawText(airportInfo, 12, posX, yOffset + 14, color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    } else {
        mapImage->drawText(airport->getDisplayID(), 14, posX, yOffset, color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    }
}

OverlayedAirport::AerodromeType OverlayedAirport::getAerodromeType() {
    if (airport->hasOnlyWaterRunways()) {
        return AerodromeType::SEAPORT;
    } else if (airport->hasOnlyHeliports()) {
        return AerodromeType::HELIPORT;
    } else if (airport->hasHardRunway()) {
        return AerodromeType::AIRPORT;
    } else {
        return AerodromeType::AIRSTRIP;
    }
}

uint32_t OverlayedAirport::getAirportColor() {
    return airport->hasControlTower() ? img::COLOR_ICAO_BLUE : img::COLOR_ICAO_MAGENTA;
}

bool OverlayedAirport::isBlob() {
    return overlayHelper->getMapDensity() > DRAW_BLOBS_ABOVE_NODE_DENSITY;
}

void OverlayedAirport::drawAirportBlob() {
    auto mw = overlayHelper->getMapWidthNM();
    if (mw != 0) {
        int radius = std::min((int)(BLOB_SIZE_DIVIDEND / mw), MAX_BLOB_SIZE);
        auto mapImage = overlayHelper->getMapImage();
        if (type != AerodromeType::AIRPORT) {
            mapImage->fillCircle(posX, posY, radius, img::COLOR_WHITE);
        } else {
            mapImage->fillCircle(posX, posY, radius, color);
        }
    }
}

int OverlayedAirport::getRunwaysDrawRadius(int zoom) {
    if (zoom < 0) zoom = overlayHelper->getZoomLevel();
    auto centre = airport->getLocation();
    auto aptYX = overlayHelper->locationToPixel(centre, zoom);
    int maxDistance = 0;
    airport->forEachRunway([this, zoom, aptYX, &maxDistance] (const std::shared_ptr<world::Runway> rwy) {
        auto loc = rwy->getLocation();
        auto rwyYX = overlayHelper->locationToPixel(loc, zoom);
        auto dy = (rwyYX.first - aptYX.first);
        auto dx = (rwyYX.second - aptYX.second);
        maxDistance = std::max(maxDistance, ((dy * dy) + (dx * dx)));
    });
    return std::sqrt(maxDistance);
}

void OverlayedAirport::drawAirportICAOGeographicRunways() {
    drawRunwayRectangles(10, color);
    drawRunwayRectangles( 3, img::COLOR_WHITE);
}

void OverlayedAirport::drawRunwayRectangles(float size, uint32_t rectColor) {
    airport->forEachRunwayPair([this, size, rectColor](const std::shared_ptr<world::Runway> rwy1, const std::shared_ptr<world::Runway> rwy2) {
        auto loc1 = rwy1->getLocation();
        auto loc2 = rwy2->getLocation();
        int y1, x1, y2, x2;
        std::tie(y1, x1) = overlayHelper->locationToPixel(loc1);
        std::tie(y2, x2) = overlayHelper->locationToPixel(loc2);
        float angleRwy = atan2((float)(y2 - y1), (float)(x2 - x1));
        float angleClockwiseCorner =     angleRwy + (45 * world::DEG_TO_RAD);
        float angleAnticlockwiseCorner = angleRwy - (45 * world::DEG_TO_RAD);
        int xc = size * cos(angleClockwiseCorner);
        int yc = size * sin(angleClockwiseCorner);
        int xa = size * cos(angleAnticlockwiseCorner);
        int ya = size * sin(angleAnticlockwiseCorner);
        auto mapImage = overlayHelper->getMapImage();
        mapImage->fillRectangle(x2 + xc, y2 + yc, x2 + xa, y2 + ya, x1 - xc, y1 - yc, x1 - xa, y1 - ya, rectColor);
    });
}

void OverlayedAirport::drawAirportICAORing() {
    auto mapImage = overlayHelper->getMapImage();

    mapImage->fillCircle(posX, posY, ICAO_RING_RADIUS, color);
    mapImage->fillCircle(posX, posY, ICAO_RING_RADIUS - 3, img::COLOR_WHITE);
    if (airport->hasOnlyHeliports()) {
        // Draw 'H'
        mapImage->drawLine(posX - 3, posY - 5, posX - 3, posY + 5, color); // Left vertical
        mapImage->drawLine(posX + 3, posY - 5, posX + 3, posY + 5, color); // Right vertical
        mapImage->drawLine(posX - 3, posY    , posX + 3, posY    , color); // Horizonatal
    } else if (airport->hasOnlyWaterRunways()) {
        // Draw anchor
        mapImage->drawLine(  posX - 3, posY - 4, posX + 3, posY - 4, color); // Top
        mapImage->drawLine(  posX    , posY - 4, posX    , posY + 4, color); // Vertical
        mapImage->drawLineAA(posX    , posY + 4, posX + 4, posY + 1, color); // Right bottom
        mapImage->drawLineAA(posX    , posY + 4, posX - 4, posY + 1, color); // Left bottom
    }
}

void OverlayedAirport::drawAirportGeographicRunways() {
    // Draw the runways as on the ground, but with slightly exaggerated widths for visibility
    airport->forEachRunwayPair([this] (const std::shared_ptr<world::Runway> rwy1, const std::shared_ptr<world::Runway> rwy2) {
        auto loc1 = rwy1->getLocation();
        auto loc2 = rwy2->getLocation();
        int px1, py1, px2, py2;
        std::tie(py1, px1) = overlayHelper->locationToPixel(loc1);
        std::tie(py2, px2) = overlayHelper->locationToPixel(loc2);
        float rwyLength = rwy1->getLength();
        float rwyWidth = rwy1->getWidth();
        if (std::isnan(rwyLength) || (rwyLength == 0)) {
            return;
        }
        if (std::isnan(rwyWidth) || (rwyWidth == 0)) {
            return;
        }
        float aspectRatio = rwyLength / (rwyWidth * 1.1);
        uint32_t surfColor = rwy1->hasHardSurface() ? img::COLOR_DARK_GREY :
            rwy1->isWater() ? img::COLOR_ICAO_BLUE : img::COLOR_DARK_GREEN;
        int xo = (px1 - px2) / aspectRatio;
        int yo = (py1 - py2) / aspectRatio;
        auto mapImage = overlayHelper->getMapImage();
        if ((xo == 0) && (yo == 0)) {
            mapImage->drawLineAA(px1, py1, px2, py2, surfColor);
        } else {
            mapImage->fillRectangle(px1 + yo, py1 - xo, px1 - yo, py1 + xo, px2 - yo, py2 + xo, px2 + yo, py2 - xo, surfColor);
        }
   });
}

void OverlayedAirport::drawAirportICAOCircleAndRwyPattern() {
    auto mapImage = overlayHelper->getMapImage();
    mapImage->fillCircle(posX, posY, ICAO_CIRCLE_RADIUS, color);
    // Scale up to fill circle - calculate pixels at higher resolution zoom level and scale down.
    int maxZoom = overlayHelper->getMaxZoomLevel();
    int cy, cx;
    std::tie(cy, cx) = overlayHelper->locationToPixel(airport->getLocation(), maxZoom);
    int maxDistance = getRunwaysDrawRadius(maxZoom);
    if (maxDistance == 0) {
        return;
    }
    float scale = (float)maxDistance / (float)(ICAO_CIRCLE_RADIUS - 4);
    cy /= scale;
    cx /= scale;

    airport->forEachRunwayPair([this, mapImage, maxZoom, cy, cx, scale](const std::shared_ptr<world::Runway> rwy1, const std::shared_ptr<world::Runway> rwy2) {
        auto loc1 = rwy1->getLocation();
        auto loc2 = rwy2->getLocation();
        int px1, py1, px2, py2;
        std::tie(py1, px1) = overlayHelper->locationToPixel(loc1, maxZoom);
        std::tie(py2, px2) = overlayHelper->locationToPixel(loc2, maxZoom);
        px1 /= scale;
        px2 /= scale;
        py1 /= scale;
        py2 /= scale;
        mapImage->drawLineAA(px1 - cx + posX, py1 - cy + posY, px2 - cx + posX, py2 - cy + posY, img::COLOR_WHITE);
    });
}

} /* namespace maps */

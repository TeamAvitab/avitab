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

#include "OverlayedMap.h"
#include "OverlayedAirport.h"
#include "OverlayedDME.h"
#include "OverlayedNDB.h"
#include "OverlayedVOR.h"
#include "OverlayedILSLocalizer.h"
#include "OverlayedFPAP.h"
#include "OverlayedWaypoint.h"
#include "OverlayedUserFix.h"
#include "AircraftIcons.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>
#include <cassert>

constexpr static bool DBG_OVERLAYS = false;

namespace maps {

OverlayedMap::OverlayedMap(std::shared_ptr<img::Stitcher> stitchedMap, std::shared_ptr<OverlayConfig> overlays):
    mapImage(stitchedMap->getPreRotatedImage()),
    tileSource(stitchedMap->getTileSource()),
    stitcher(stitchedMap),
    overlayConfig(overlays),
    copyrightStamp("Inconsolata.ttf")
{
    /*
     * This class is injected between a stitcher and its client:
     * Whenever the stitcher redraws, we draw our overlays and then
     * call the client.
     */

    invalidateLastClick();

    copyrightStamp.setSize(16);
    copyrightStamp.setColor(0x404040);
    copyrightStamp.setText(tileSource->getCopyrightInfo());

    stitcher->setPreRotateCallback([this] () {
        if (!overlaysOnScreen()) {
            drawOverlays();
        }
    });
    stitcher->setRedrawCallback([this] () {
        if (overlaysOnScreen()) {
            drawOverlays();
        }
        copyrightStamp.applyStamp(*(stitcher->getTargetImage()), 0);
        if (tileSource->isDocumentSource() && tileSource->supportsWorldCoords()) {
            drawCompass();
        }
        if (onOverlaysDrawn) {
            onOverlaysDrawn();
        }
    });

    for (int angle = 0; angle < 360; angle++) {
        sinTable[angle] = std::sin(angle * world::DEG_TO_RAD);
        cosTable[angle] = std::cos(angle * world::DEG_TO_RAD);
    }

    planeIcons = AircraftIcons::factory(overlayConfig->colorOtherAircraftBelow, overlayConfig->colorOtherAircraftSame, overlayConfig->colorOtherAircraftAbove);

    overlayNodeCache = std::make_shared<NavNodeToOverlayMap>();
}

void OverlayedMap::setRedrawCallback(OverlaysDrawnCallback cb) {
    onOverlaysDrawn = cb;
}

void OverlayedMap::setGetRouteCallback(GetRouteCallback cb) {
    getRoute = cb;
}

void OverlayedMap::setGetMagneticVariationsCallback(GetMagneticVariationsCallback cb) {
    getMagneticVariations = cb;
}

void OverlayedMap::setNavWorld(std::shared_ptr<navdb::NavDatabase> world) {
    navWorld = world;
}

// Returns true if the map was moved and redrawn
bool OverlayedMap::centerOnLocation(const world::Location &loc) {
    if (!tileSource->supportsWorldCoords()) {
        return false;
    }

    int zoomLevel = stitcher->getZoomLevel();
    auto centerXY = tileSource->worldToXY(loc, zoomLevel);
    int page = stitcher->getCurrentPage();
    if (tileSource->isTileValid(page, centerXY.x, centerXY.y, zoomLevel)) {
        return stitcher->setCenter(centerXY.x, centerXY.y);
    }
    return false;
}

void OverlayedMap::setPlaneLocations(std::vector<world::Position> &locs, bool followPlane) {
    if (!tileSource->supportsWorldCoords()) {
        return;
    }

    bool movement = false;
    for (size_t i = 0; !movement && (i < locs.size()); ++i) {
        if (i < planeLocations.size()) {
            double deltaLat = std::abs(locs[i].latDegrees() - planeLocations[i].latDegrees());
            double deltaLon = std::abs(locs[i].lonDegrees() - planeLocations[i].lonDegrees());
            double deltaHeading = std::abs(locs[i].hdgDegrees() - planeLocations[i].hdgDegrees());
            movement |= (deltaLat > 0.0000001 || deltaLon > 0.0000001 || deltaHeading > 0.1);
        } else {
            movement = true;
        }
    }
    planeLocations = locs;

    // rotate and centre first, so the map is drawn only once per update
    bool rotated = applyTrackUpRotation();
    if (followPlane && !planeLocations.empty() && centerOnLocation(planeLocations[0])) {
        return; // moving the centre has already redrawn the map
    }
    // a changed rotation redraws as well
    if (rotated || movement) {
        stitcher->updateImage();
    }
}

void OverlayedMap::setTrackUp(bool enable) {
    trackUp = enable;
    invalidateLastClick();
    if (applyTrackUpRotation()) {
        stitcher->updateImage();
    }
}

// Returns true if the rotation or position of the map changed
bool OverlayedMap::applyTrackUpRotation() {
    double rotation = 0;
    if (trackUp && !planeLocations.empty()) {
        // whole degrees, so a steady heading doesn't redraw the map all the time
        // the tile source's own offset: getNorthOffset() already includes this rotation
        rotation = -std::round(planeLocations[0].hdgDegrees() + tileSource->getNorthOffsetAngle());
    }
    // in track-up mode keep the aircraft near the bottom to see more of what is ahead
    int offsetY = trackUp ? stitcher->getTargetImage()->getHeight() / 3 : 0;
    bool rotated = stitcher->setViewRotation(rotation);
    bool moved = stitcher->setViewOffsetY(offsetY);
    return rotated || moved;
}

world::Location OverlayedMap::getCenterLocation() {
    return pixelToLocation(mapImage->getWidth() / 2, mapImage->getHeight() / 2);
}

float OverlayedMap::getVerticalRange() const {
    auto rangeLat = (neCorner.latDegrees() - swCorner.latDegrees())
                * stitcher->getTargetImage()->getHeight() / mapImage->getHeight();
    auto rangeKM = rangeLat * world::LAT_TO_KM;
    return (float)rangeKM * 1000;
}

void OverlayedMap::mouseClick(int x, int y)
{
    if ((x < 0) || (y < 0)) return;
    lastClickX = x; lastClickY = y;
    if (!overlaysOnScreen()) {
        stitcher->convertSourceImageToRenderedCoords(lastClickX, lastClickY);
    }
}

void OverlayedMap::pan(int dx, int dy) {
    stitcher->pan(dx, dy);
    invalidateLastClick();
}

void OverlayedMap::zoomIn() {
    int zoomLevel = stitcher->getZoomLevel();
    stitcher->setZoomLevel(zoomLevel + 1);
    invalidateLastClick();
}

void OverlayedMap::zoomOut() {
    int zoomLevel = stitcher->getZoomLevel();
    stitcher->setZoomLevel(zoomLevel - 1);
    invalidateLastClick();
}

void OverlayedMap::zoomAt(bool in, int dx, int dy) {
    int oldZoom = stitcher->getZoomLevel();
    int page = stitcher->getCurrentPage();
    auto center = stitcher->getCenter();
    auto oldDim = tileSource->getTileDimensions(oldZoom);

    // the pointer offset on the screen -> offset in the unrotated map image (same mapping as Stitcher::pan)
    double ux = dx, uy = dy;
    switch (stitcher->getRotation()) {
    case 90:  ux = dy;  uy = -dx; break;
    case 180: ux = -dx; uy = -dy; break;
    case 270: ux = -dy; uy = dx;  break;
    }

    // the map point under the pointer, in tile units at the old zoom level
    double ptX = center.x + ux / oldDim.x;
    double ptY = center.y + uy / oldDim.y;

    if (in) {
        zoomIn();
    } else {
        zoomOut();
    }

    int newZoom = stitcher->getZoomLevel();
    if (newZoom == oldZoom) {
        return; // already at the zoom limit
    }

    // put that point under the pointer again: centre = point - pointer offset
    auto newPt = tileSource->transformZoomedPoint(page, ptX, ptY, oldZoom, newZoom);
    auto newDim = tileSource->getTileDimensions(newZoom);
    double cx = newPt.x - ux / newDim.x;
    double cy = newPt.y - uy / newDim.y;
    tileSource->constrainXY(cx, cy, newZoom);
    stitcher->setCenter(cx, cy);
}

void OverlayedMap::updateImage() {
    stitcher->updateImage();
}

void OverlayedMap::doWork() {
    stitcher->doWork();
}

void OverlayedMap::drawOverlays() {
    static bool skippedFirst = false;
    if (!skippedFirst) {
        // ignore the first call as the stitcher has not been fully initialised
        skippedFirst = true;
        return;
    }
    auto img = overlayImage();
    if ((img->getWidth() == 0) || (img->getHeight() == 0)) {
        return;
    }
    if (tileSource->supportsWorldCoords()) {
        updateMapAttributes();
        drawRoute();
        drawNavWorldOverlays();
        drawScale();
        drawOtherAircraftOverlay();
        drawAircraftOverlay();
    }
    drawCalibrationOverlay();
}

void OverlayedMap::drawAircraftOverlay() {
    if (!overlayConfig->drawMyAircraft || planeLocations.empty()) {
        return;
    }

    int py, px;
    double angle;
    auto icon = planeIcons->getUserPlaneIcon();
    if (trackUp) {
        px = overlayImage()->getWidth() / 2 - icon->getWidth() / 2;
        py = overlayImage()->getHeight() / 2 + stitcher->getViewOffsetY() - icon->getHeight() / 2;
        angle = 0;
    }
    else {
        std::tie(py, px) = locationToPixel(planeLocations[0]);
        px -= icon->getWidth() / 2;
        py -= icon->getHeight() / 2;
        angle = planeLocations[0].hdgDegrees() + getNorthOffset();
    }
    overlayImage()->blendImage(*icon, px, py, angle);
}

void OverlayedMap::drawOtherAircraftOverlay() {
    auto img = overlayImage();
    if (!overlayConfig->drawOtherAircraft || (planeLocations.size() < 2)) {
        return;
    }

    int px, py;
    for (size_t i = 1; i < planeLocations.size(); ++i) {
        bool isAbove = (planeLocations[i].alt_metres > (planeLocations[0].alt_metres + 30));
        bool isBelow = (planeLocations[i].alt_metres < (planeLocations[0].alt_metres - 30));
        std::tie(py, px) = locationToPixel(planeLocations[i]);
        auto icon = planeIcons->getOtherPlaneIcon(isBelow, isAbove);
        img->blendImage(*icon, px - (icon->getWidth() / 2), py - (icon->getHeight() / 2), planeLocations[i].hdgDegrees() + getNorthOffset());
        unsigned int flightLevel = static_cast<unsigned int>(planeLocations[i].altFeet() + 50.0) / 100.0;
        std::string flText = "---";
        flText[0] = '0' + (flightLevel / 100) % 10;
        flText[1] = '0' + (flightLevel / 10) % 10;
        flText[2] = '0' + (flightLevel / 1) % 10;
        if (isAbove) {
            img->drawText(flText, 12, px, py - 17, img::COLOR_BLACK, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
        }
        if (isBelow) {
            img->drawText(flText, 12, px, py + 7, img::COLOR_BLACK, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
        }
    }
}

void OverlayedMap::drawCalibrationOverlay() {
    if (calibrationStep == 0) {
        return;
    }

    uint32_t color = img::COLOR_WHITE;
    if (calibrationStep == 1) {
        color = img::COLOR_LIGHT_RED;
    } else if (calibrationStep == 2) {
        color = img::COLOR_BLUE;
    } else {
        color = img::COLOR_DARK_GREEN;
    }
    auto img = overlayImage();
    int centerX = img->getWidth() / 2;
    int centerY = img->getHeight() / 2 + (overlaysOnScreen() ? stitcher->getViewOffsetY() : 0);
    int r = 10;

    img->drawLine(centerX - r, centerY - r, centerX + r, centerY - r, color);
    img->drawLine(centerX - r, centerY + r, centerX + r, centerY + r, color);
    img->drawLine(centerX - r, centerY - r, centerX - r, centerY + r, color);
    img->drawLine(centerX + r, centerY - r, centerX + r, centerY + r, color);

    img->drawLine(centerX - r / 2, centerY, centerX + r / 2, centerY, color);
    img->drawLine(centerX, centerY + r / 2, centerX, centerY - r / 2, color);
}

void OverlayedMap::drawNavWorldOverlays() {
    // if the navigation database has been superseded then switch to the latest one
    if (navWorld->status() == navdb::NavDatabase::NavStatus::SUPERSEDED) {
        navWorld = navWorld->latest();
        assert(navWorld);
    }

    int nodeFilter = 0;
    nodeFilter |= (overlayConfig->drawAirports) ? navdb::NavDatabase::VISIT_TOWERED_AIRPORTS : 0;
    nodeFilter |= (overlayConfig->drawAirstrips || overlayConfig->drawHeliportsSeaports) ? navdb::NavDatabase::VISIT_OTHER_AIRPORTS : 0;
    nodeFilter |= (overlayConfig->drawWaypoints) ? navdb::NavDatabase::VISIT_FIXES : 0;
    nodeFilter |= (overlayConfig->drawVORs || overlayConfig->drawNDBs || overlayConfig->drawILSs || overlayConfig->drawFPAPs) ? navdb::NavDatabase::VISIT_NAVAIDS : 0;
    nodeFilter |= (overlayConfig->drawPOIs || overlayConfig->drawVRPs || overlayConfig->drawMarkers) ? navdb::NavDatabase::VISIT_USER_FIXES : 0;
    if (!nodeFilter) {
        return;
    }

    // Don't overlay anything if zoomed out beyond half of the globe
    auto leftEdge = swCorner.lonDegrees();
    auto rightEdge = neCorner.lonDegrees();
    auto widthDegrees = rightEdge - leftEdge;
    if (widthDegrees > 180) {
        return;
    }
    if ((rightEdge < leftEdge) && ((rightEdge + 180) > leftEdge)) {
        // meridian-wrapping version
        return;
    }

    // Extend the area of the search to ensure that any NAV data that might be partially visible
    // at the edges of the window will be included, even if the specific NAV item ends up being
    // entirely off-screen.
    double marginDegrees = (double)MAX_ILS_RANGE_NM / MAX_NM_PER_DEGREE;
    auto searchMin = world::Location::fromGCS(swCorner.latDegrees() - marginDegrees, leftEdge - marginDegrees);
    auto searchMax = world::Location::fromGCS(neCorner.latDegrees() + marginDegrees, rightEdge + marginDegrees);

    // Find out what is the likely maximum density of the area being shown, and use this value
    // to configure filters for the node search and graphic/text drawing styles.
    maxNodeDensity = navWorld->maxDensity(searchMin, searchMax);
    LOG_INFO(DBG_OVERLAYS,"Estimating %d nodes in (%0.2f,%0.2f) -> (%0.2f,%0.2f)",
                    maxNodeDensity, searchMin.lonDegrees(), searchMin.latDegrees(),
                    searchMax.lonDegrees(), searchMax.latDegrees());
    if (maxNodeDensity > MAX_VISIT_OBJECTS_IN_FRAME) {
        return;
    }
    if (maxNodeDensity > DENSITY_LIMIT_AIRFIELDS) {
        nodeFilter &= ~(navdb::NavDatabase::VISIT_OTHER_AIRPORTS);
    }
    if (maxNodeDensity > DENSITY_LIMIT_NAVAIDS) {
        nodeFilter &= ~(navdb::NavDatabase::VISIT_NAVAIDS);
    }
    if (maxNodeDensity > DENSITY_LIMIT_FIXES) {
        nodeFilter &= ~(navdb::NavDatabase::VISIT_FIXES);
    }
    if (mapWidthNM > MAPWIDTH_LIMIT_USERFIXES) {
        nodeFilter &= ~(navdb::NavDatabase::VISIT_USER_FIXES);
    }
    if (!nodeFilter) {
        return;
    }

    // All the short-circuit tests have been done now, and we know that at least some
    // overlays should be displayed. Get the qualifying NAV nodes from the world data
    // and reuse the associated overlay where available, or create a new overlay if not.

    int reusedOverlays = 0; // this is only used for cache hit statistics
    std::shared_ptr<NavNodeToOverlayMap> nodes = std::make_shared<NavNodeToOverlayMap>();

    navWorld->visitNodes(searchMin, searchMax,
                        [this, &reusedOverlays, nodes] (const navdb::Node *node) {
                            // coarse filtering has been done by the NAV database, but
                            // further detailed filtering is needed here
                            if (!isOverlayConfigured(node)) return;
                            // did we already see this NAV item in the previous frame?
                            // if so then we can just reuse its overlay node
                            std::shared_ptr<OverlayedNode> on;
                            auto i = overlayNodeCache->find(node);
                            if (i == overlayNodeCache->end()) {
                                on = makeOverlayedNode(node);
                            } else {
                                on = (*overlayNodeCache)[node];
                                ++reusedOverlays;
                            }
                            if (on) {
                                (*nodes)[node] = on;
                                on->configure(*(overlayConfig.get()), node->getLocation());
                            }
                        },
                        nodeFilter);

    fetchMagneticVariations(*nodes);

    // decide whether all nodes should show their detailed text, this will be based
    // on the reported node density
    const bool showDetailedText = (maxNodeDensity < DENSITY_LIMIT_DETAILED_TEXT);

    // work out which highlights are active, and initialise them
    for (size_t i = 0; i < NUM_HIGHLIGHT_NODES; ++i) {
        highlights[i].reset();
    }
    if (!showDetailedText) {
        if (lastClickX >= 0) {
            highlights[LAST_CLICK].activate(lastClickX, lastClickY);
        }
        if (overlayConfig->drawMyAircraft && !planeLocations.empty()) {
            int x, y;
            std::tie(y,x) = locationToPixel(planeLocations[0]);
            highlights[USER_PLANE].activate(x, y);
        }
        auto img = overlayImage();
        highlights[MAP_CENTER].activate(img->getWidth() / 2, img->getHeight() / 2 + (overlaysOnScreen() ? stitcher->getViewOffsetY() : 0));
    }

    // split the collection of nodes into fixes and aerodromes, and find the ones to be highlighted
    std::vector<std::shared_ptr<OverlayedNode>> fixes;
    std::vector<std::shared_ptr<OverlayedNode>> aerodromes;
    for (auto on: *nodes) {
        if (on.second->isAirfield()) {
            aerodromes.push_back(on.second);
        } else {
            fixes.push_back(on.second);
        }
        for (size_t i = 0; i < NUM_HIGHLIGHT_NODES; ++i) {
            highlights[i].update(on.second);
        }
    }

    // the nearest nodes have been identified, now mark them as selected
    for (size_t i = 0; i < NUM_HIGHLIGHT_NODES; ++i) {
        highlights[i].select();
    }

    // Render the list of visible OverlayedNodes:
    // Fix graphics, aerodrome graphics, then fix text and aerodrome text, then highlighted text
    for (auto on: fixes) {
        on->drawGraphic();
    }
    for (auto on: aerodromes) {
        on->drawGraphic();
    }
    if (maxNodeDensity < DENSITY_LIMIT_SHOW_TEXT) {
        for (auto on: fixes) {
            if (!on->isHighlighted()) on->drawText(showDetailedText);
        }
        for (auto on: aerodromes) {
            if (!on->isHighlighted()) on->drawText(showDetailedText);
        }
    }
    for (size_t i = 0; i < NUM_HIGHLIGHT_NODES; ++i) {
        highlights[i].highlight();
    }

    LOG_INFO(DBG_OVERLAYS, "zoom = %2d, nm/pix = %0.3f, mapWidth = %0.1f nm, maxNodes = %d, actual = %d (%d/%d from cache)",
        stitcher->getZoomLevel(), mapScaleNMperPixel, mapWidthNM, maxNodeDensity, nodes->size(), reusedOverlays, overlayNodeCache->size());

    // Keep this frame's overlays for next time. The previous cache will be disposed of and
    // and the overlay shared pointers released, which will result in the overlay being destroyed
    // if it wasn't reused in this frame.
    overlayNodeCache = nodes;
}

int OverlayedMap::getMapDensity() const {
    return maxNodeDensity;
}

double OverlayedMap::getMapWidthNM() const {
    return mapWidthNM;
}

void OverlayedMap::drawScale() {
    auto img = overlayImage();
    bool useFeet = (mapScaleNMperPixel < 0.005);
    std::string units = useFeet ? "ft" : "nm";
    double perPixel = useFeet ? (mapScaleNMperPixel * 6076) : mapScaleNMperPixel;
    double maxRange = 300 * perPixel;
    double step = std::pow(10, (std::floor(std::log10(maxRange))));
    double rangeToShow = step;
    if ((rangeToShow * 5) < maxRange) {
        rangeToShow *= 5;
    } else if ((rangeToShow * 2) < maxRange) {
        rangeToShow *= 2;
    }
    LOG_INFO(DBG_OVERLAYS, "zoom = %d, %f%s/pixel, maxRange = %f%s, step = %f%s, show = %f%s",
        stitcher->getZoomLevel(),  perPixel, units.c_str(),  maxRange, units.c_str(),
        step, units.c_str(),  rangeToShow, units.c_str());
    int x = 5;
    int y = 195;
    if (overlaysOnScreen()) {
        // x, y are relative to the pre-rotated image of a north-up map, a square with the target
        // image centred in it; keep the scale where it is in that case (top left)
        int size = std::max(img->getWidth(), img->getHeight());
        x -= (size - img->getWidth()) / 2;
        y -= (size - img->getHeight()) / 2;
    }
    if (perPixel == 0) {
        return;
    }
    int lineLength = rangeToShow / perPixel;
    img->drawLine(x, y,  x + lineLength, y,  img::COLOR_BLACK);
    for (int tick = 0; tick <= rangeToShow; tick += step) {
        img->drawLine(x + (tick / perPixel), y,  x + (tick / perPixel), y + 10,  img::COLOR_BLACK);
    }
    std::string text = std::to_string(int(rangeToShow)) + units;
    int xtext = x + lineLength + 2;
    img->drawText(text, 12, xtext, y, img::COLOR_BLACK, img::COLOR_TRANSPARENT_WHITE, img::Align::LEFT);
}

void OverlayedMap::drawCompass() {
    int cx = stitcher->getTargetImage()->getWidth() - 30;
    int cy = 30;
    auto rotatedImage = stitcher->getTargetImage();
    int compassDir = (int)(stitcher->getRotation() + stitcher->getViewRotation() + tileSource->getNorthOffsetAngle()) % 360;
    rotatedImage->drawCircle(cx, cy, 20, img::COLOR_RED);
    double xt, yt, xm, ym, xp, yp;
    fastPolarToCartesian(20, compassDir,      xt, yt);
    fastPolarToCartesian( 5, compassDir + 90, xm, ym);
    fastPolarToCartesian( 5, compassDir - 90, xp, yp);
    rotatedImage->drawLineAA(cx - xt, cy - yt, cx + xt, cy + yt, img::COLOR_RED);
    rotatedImage->drawLineAA(cx + xt, cy + yt, cx + xp, cy + yp, img::COLOR_RED);
    rotatedImage->drawLineAA(cx + xt, cy + yt, cx + xm, cy + ym, img::COLOR_RED);
}

std::pair<int, int> OverlayedMap::locationToPixel(const world::Location& loc) const {
    return locationToPixel(loc, stitcher->getZoomLevel());
}

std::pair<int, int>  OverlayedMap::locationToPixel(const world::Location& loc, int zoomLevel) const {
    auto mapWidth = tileSource->getPageDimensions(0, zoomLevel).x;
    auto dim = tileSource->getTileDimensions(zoomLevel);

    // Center tile num
    auto centerXY = stitcher->getCenter();

    // Target tile num
    auto tileXY = tileSource->worldToXY(loc, zoomLevel);

    // Adjust for wrapping at the -180/180 meridian
    if (tileXY.x > (centerXY.x + (mapWidth / 2))) {
        tileXY.x -= mapWidth;
    } else if (tileXY.x < (centerXY.x - (mapWidth / 2))) {
        tileXY.x += mapWidth;
    }

    double px = mapImage->getWidth() / 2.0 + (tileXY.x - centerXY.x) * dim.x;
    double py = mapImage->getHeight() / 2.0 + (tileXY.y - centerXY.y) * dim.y;

    if (overlaysOnScreen()) {
        // from the pre-rotated image to the rotated target image
        double a = (stitcher->getRotation() + stitcher->getViewRotation()) * world::DEG_TO_RAD;
        double rx = px - mapImage->getWidth() / 2.0;
        double ry = py - mapImage->getHeight() / 2.0;
        auto target = stitcher->getTargetImage();
        px = target->getWidth() / 2.0 + rx * std::cos(a) - ry * std::sin(a);
        py = target->getHeight() / 2.0 + stitcher->getViewOffsetY() + rx * std::sin(a) + ry * std::cos(a);
    }

    return std::make_pair((int) std::lround(py), (int) std::lround(px));
}

void OverlayedMap::updateMapAttributes()
{
    // Update the corner positions
    swCorner = pixelToLocation(0, mapImage->getHeight() - 1);
    neCorner = pixelToLocation(mapImage->getWidth() - 1, 0);

    // Calculate scaling from a hybrid of horizontal and vertical axes
    double diagonalPixels = sqrt(pow(mapImage->getWidth(), 2) + pow(mapImage->getHeight(), 2));
    double kmPerPixel = (swCorner.surfaceDistanceTo(neCorner) / diagonalPixels) / 1000;
    mapScaleNMperPixel = kmPerPixel * world::KM_TO_NM;
    mapWidthNM = mapScaleNMperPixel * mapImage->getWidth();
}

world::Location OverlayedMap::pixelToLocation(int px, int py) const {
    int zoomLevel = stitcher->getZoomLevel();
    auto dim = tileSource->getTileDimensions(zoomLevel);

    auto centerXY = stitcher->getCenter();

    double x = centerXY.x + (px - mapImage->getWidth() / 2.0) / dim.x;
    double y = centerXY.y + (py - mapImage->getHeight() / 2.0) / dim.y;

    return tileSource->xyToWorld(x, y, zoomLevel);
}

float OverlayedMap::cosDegrees(int angleDegrees) const {
    // If you're going to use this, be sure that an integer angle is suitable.
    int angle = angleDegrees % 360;
    if (angle < 0) {
        angle += 360;
    }
    return cosTable[angle];
}

float OverlayedMap::sinDegrees(int angleDegrees) const {
    // If you're going to use this, be sure that an integer angle is suitable.
    int angle = angleDegrees % 360;
    if (angle < 0) {
        angle += 360;
    }
    return sinTable[angle];
}

void OverlayedMap::fastPolarToCartesian(float radius, int angleDegrees, double& x, double& y) const {
    // If you're going to use this, be sure that an integer angle is suitable.
    x = sinDegrees(angleDegrees) * radius;
    y = -cosDegrees(angleDegrees) * radius; // 0 degrees is up, decreasing y values
}

void OverlayedMap::polarToCartesian(float radius, float angleDegrees, double& x, double& y) {
    x = std::sin(angleDegrees * world::DEG_TO_RAD) * radius;
    y = -std::cos(angleDegrees * world::DEG_TO_RAD) * radius; // 0 degrees is up, decreasing y values
}

bool OverlayedMap::isAreaVisible(int xmin, int ymin, int xmax, int ymax) const {
    auto img = overlayImage();
    return (xmax > 0) && (xmin < img->getWidth()) &&
           (ymax > 0) && (ymin < img->getHeight());
}

int OverlayedMap::getZoomLevel() const {
    return stitcher->getZoomLevel();
}

int OverlayedMap::getMaxZoomLevel() const {
    return tileSource->getMaxZoomLevel();
}

std::map<std::pair<double, double>, float> OverlayedMap::magneticVariationCache;

void OverlayedMap::fetchMagneticVariations(const NavNodeToOverlayMap &nodes) {
    if (!getMagneticVariations) {
        return;
    }

    std::vector<world::Location> locs;
    for (auto &n: nodes) {
        if (auto fpap = std::dynamic_pointer_cast<OverlayedFPAP>(n.second)) {
            auto &loc = fpap->getCourseLocation();
            if (std::isnan(getMagneticVariation(loc))) {
                locs.push_back(loc);
            }
        }
    }
    if (locs.empty()) {
        return;
    }

    try {
        auto magVars = getMagneticVariations(locs);
        for (size_t i = 0; i < std::min(locs.size(), magVars.size()); ++i) {
            magneticVariationCache[{locs[i].lat(), locs[i].lon()}] = magVars[i];
        }
    } catch (const std::exception &e) {
        // e.g. the sim driver is stopped, leave the values unknown and retry on the next redraw
        static bool logged = false;
        if (!logged) {
            LOG_ERROR("Couldn't get magnetic variations: %s", e.what());
            logged = true;
        }
    }
}

float OverlayedMap::getMagneticVariation(const world::Location &loc) const {
    auto i = magneticVariationCache.find({loc.lat(), loc.lon()});
    return (i == magneticVariationCache.end()) ? std::nanf("") : i->second;
}

double OverlayedMap::getNorthOffset() const {
    double offset = tileSource->getNorthOffsetAngle();
    if (overlaysOnScreen()) {
        // headings are drawn in the rotated frame
        offset += stitcher->getRotation() + stitcher->getViewRotation();
    }
    return offset;
}

bool OverlayedMap::overlaysOnScreen() const {
    return trackUp && tileSource->supportsWorldCoords();
}

std::shared_ptr<img::Image> OverlayedMap::overlayImage() const {
    return overlaysOnScreen() ? stitcher->getTargetImage() : mapImage;
}

bool OverlayedMap::isCalibrated() const {
    return tileSource->supportsWorldCoords();
}

std::string OverlayedMap::getCalibrationReport() const {
    return tileSource->getCalibrationReport();
}

void OverlayedMap::beginCalibration() {
    calibrationStep = 1;
    updateImage();
}

void OverlayedMap::setCalibrationPoint1(const world::Location &loc) {
    auto center = stitcher->getCenter();
    tileSource->attachCalibration1(center.x, center.y, loc.latDegrees(), loc.lonDegrees(), stitcher->getZoomLevel());

    calibrationStep = 2;
    updateImage();
}

void OverlayedMap::setCalibrationPoint2(const world::Location &loc) {
    auto center = stitcher->getCenter();
    tileSource->attachCalibration2(center.x, center.y, loc.latDegrees(), loc.lonDegrees(), stitcher->getZoomLevel());

    calibrationStep = 3;
    updateImage();
}

void OverlayedMap::setCalibrationPoint3(const world::Location &loc) {
    auto center = stitcher->getCenter();
    tileSource->attachCalibration3Point(center.x, center.y, loc.latDegrees(), loc.lonDegrees(), stitcher->getZoomLevel());
    calibrationStep = 0;
    updateImage();
}

void OverlayedMap::setCalibrationAngle(double angle) {
    tileSource->attachCalibration3Angle(angle);
    calibrationStep = 0;
    updateImage();
}

int OverlayedMap::getCalibrationStep() const {
    return calibrationStep;
}

std::shared_ptr<img::Image> OverlayedMap::getMapImage() {
    return overlayImage();
}

void OverlayedMap::drawRoute() {
    auto route = getRoute();
    if (!route || !overlayConfig->drawRoute) {
        return;
    }
    if (!overlayedRoute) {
        overlayedRoute = std::make_unique<OverlayedRoute>(static_cast<IOverlayHelper *>(this));
    }
    overlayedRoute->draw(route);
}

bool maps::OverlayedMap::isOverlayConfigured(const navdb::Node *nn) const {
    // do further filtering of the broader groups returned by the NAV db
    if (auto a = dynamic_cast<const navdb::Airport *>(nn)) {
        // use config settings to filter heliports/seaports and airfields
        if (a->hasOnlyHeliports() || a->hasOnlyWaterRunways()) {
            return overlayConfig->drawHeliportsSeaports;
        } else if (!a->hasHardRunway()) {
            return overlayConfig->drawAirstrips;
        }
    } else if (auto nf = dynamic_cast<const navdb::NavFix *>(nn)) {
        bool draw = overlayConfig->drawWaypoints;
        if (nf->hasNavaid()) {
            draw |= (nf->getNDB() && overlayConfig->drawNDBs);
            draw |= (nf->getVOR() && overlayConfig->drawVORs);
            draw |= (nf->getDME() && overlayConfig->drawVORs);
            draw |= (nf->getILSLocalizer() && overlayConfig->drawILSs);
            draw |= (nf->getFPAP() && overlayConfig->drawFPAPs);
        }
        return draw;
    } else if (auto uf = dynamic_cast<const navdb::UserFix *>(nn)) {
        if (uf->getType() == navdb::UserFixType::VRP) {
            return overlayConfig->drawVRPs;
        } else if (uf->getType() == navdb::UserFixType::POI) {
            return overlayConfig->drawPOIs;
        } else if (uf->getType() == navdb::UserFixType::MARKER) {
            return overlayConfig->drawMarkers;
        }
    }
    return true;
}

 // REFACTOR - this needs changing to use shared_ptr before SQL NAV data is introduced.
std::shared_ptr<OverlayedNode> OverlayedMap::makeOverlayedNode(navdb::Node const *nn) {
    std::shared_ptr<OverlayedNode> on;
    if (auto a = dynamic_cast<const navdb::Airport *>(nn)) {
        on = std::make_shared<OverlayedAirport>(static_cast<IOverlayHelper *>(this), a);
    } else if (auto nf = dynamic_cast<const navdb::NavFix *>(nn)) {
        if (nf->getILSLocalizer()) {
            on = std::make_shared<OverlayedILSLocalizer>(static_cast<IOverlayHelper *>(this), nf);
        } else if (nf->getFPAP()) {
            on = std::make_shared<OverlayedFPAP>(static_cast<IOverlayHelper *>(this), nf);
        } else if (nf->getVOR()) {
            on = std::make_shared<OverlayedVOR>(static_cast<IOverlayHelper *>(this), nf);
        } else if (nf->getDME()) {
            on = std::make_shared<OverlayedDME>(static_cast<IOverlayHelper *>(this), nf);
        } else if (nf->getNDB()) {
            on = std::make_shared<OverlayedNDB>(static_cast<IOverlayHelper *>(this), nf);
        } else {
            on = std::make_shared<OverlayedWaypoint>(static_cast<IOverlayHelper *>(this), nf);
        }
    } else if (auto uf = dynamic_cast<const navdb::UserFix *>(nn)) {
        on = std::make_shared<OverlayedUserFix>(static_cast<IOverlayHelper *>(this), uf);
    }
    return on;
}

} /* namespace maps */

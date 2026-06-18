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
#include <sstream>
#include <stdexcept>
#include <cmath>
#include "EPSGSource.h"
#include "platform/Platform.h"

namespace maps {

EPSGSource::EPSGSource(const std::filesystem::path& tilePathUTF8):
    tilePath(tilePathUTF8)
{
    auto dir = platform::readDirectory(tilePathUTF8);
    for (auto &entry: dir) {
        if (entry.isDirectory) {
            int num = std::stoi(entry.utf8Name);
            if (num > maxLevel) {
                maxLevel = num;
            }
            if (num < minLevel) {
                minLevel = num;
            }
        }
    }
}

int EPSGSource::getMinZoomLevel() {
    return minLevel;
}

int EPSGSource::getMaxZoomLevel() {
    return maxLevel;
}

int EPSGSource::getInitialZoomLevel() {
    return minLevel + (maxLevel - minLevel) / 2;
}

img::Point<double> EPSGSource::suggestInitialCenter(int page) {
    return img::Point<double>{0, 0};
}

bool EPSGSource::supportsWorldCoords() {
    return true;
}

img::Point<int> EPSGSource::getTileDimensions(int zoom) {
    return img::Point<int>{256, 256};
}

img::Point<double> EPSGSource::transformZoomedPoint(int page, double oldX, double oldY, int oldZoom, int newZoom) {
    if (oldZoom == newZoom) {
        return img::Point<double>{oldX, oldY};
    }

    int diff = newZoom - oldZoom;
    if (diff > 0) {
        oldX *= (1 << diff);
        oldY *= (1 << diff);
    } else {
        oldX /= (1 << -diff);
        oldY /= (1 << -diff);
    }
    return img::Point<double>{oldX, oldY};
}

void EPSGSource::cancelPendingLoads() {
}

void EPSGSource::resumeLoading() {
}

int EPSGSource::getPageCount() {
    return 1;
}

img::Point<int> EPSGSource::getPageDimensions(int page, int zoom) {
    return img::Point<int>{0, 0};
}

bool EPSGSource::isTileValid(int page, int x, int y, int zoom) {
    if (page != 0) {
        return false;
    }

    uint32_t endXY = 1 << zoom;

    if (y < 0 || (uint32_t) y >= endXY) {
        // y isn't repeating, so don't correct it
        return false;
    }

    if (x < 0 || (uint32_t) x >= endXY) {
        // disable wrapping for now because it is broken on higher layers
        return false;
    }

    return true;
}

std::string EPSGSource::getUniqueTileName(int page, int x, int y, int zoom) {
    if (!isTileValid(page, x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    std::ostringstream nameStream;
    nameStream << zoom << "/" << x << "/" << y << ".png";
    return nameStream.str();
}

std::unique_ptr<img::Image> EPSGSource::loadTileImage(int page, int x, int y, int zoom) {
    auto img = std::make_unique<img::Image>();
    img->loadImageFile(tilePath / std::filesystem::u8path(getUniqueTileName(page, x, y, zoom)));

    return img;
}

img::Point<double> EPSGSource::worldToXY(const world::Location &loc, int zoom) {
    auto p = loc.toTileYX(zoom);
    return img::Point<double>{p.second, p.first};
}

world::Location EPSGSource::xyToWorld(double x, double y, int zoom) {
    return world::Location::fromTileYX(y, x, zoom);
}

} /* namespace maps */

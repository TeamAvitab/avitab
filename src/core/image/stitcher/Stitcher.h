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
#include <functional>
#include "TileSource.h"
#include "TileCache.h"
#include "image/Image.h"

namespace img {

class Stitcher {
public:
    using RedrawCallback = std::function<void(void)>;
    using PreRotateCallback = std::function<void(void)>;

    Stitcher(std::shared_ptr<Image> dstImage, std::shared_ptr<TileSource> source);
    void setCacheDirectory(const std::filesystem::path &utf8Path);
    void setPreRotateCallback(PreRotateCallback cb);
    void setRedrawCallback(RedrawCallback cb);

    bool setCenter(double x, double y); // true if the centre moved and the image was redrawn
    img::Point<double> getCenter() const;

    int getCurrentPage() const;
    int getPageCount() const;
    bool nextPage();
    bool prevPage();

    void pan(int dx, int dy);
    void setZoomLevel(int level);
    int getZoomLevel();

    void invalidateCache();
    void updateImage();
    void doWork();

    int getRotation() const;
    void rotateRight();
    // Free rotation (clockwise degrees) on top of the 90 degree steps. Returns true if it changed;
    // the caller must call updateImage() then. While non-zero the pre-rotated image is enlarged
    // so that it still covers the target image at any angle.
    bool setViewRotation(double degrees);
    double getViewRotation() const;
    // Shows the centre of the map this many pixels below the centre of the target image
    // (e.g. to keep the aircraft near the bottom). Returns true if it changed.
    bool setViewOffsetY(int pixels);
    int getViewOffsetY() const;

    std::shared_ptr<Image> getPreRotatedImage();
    std::shared_ptr<Image> getTargetImage();
    std::shared_ptr<TileSource> getTileSource();
    void convertSourceImageToRenderedCoords(int &x, int &y);

private:
    int page = 0;
    Image emptyTile, errorTile, loadingTile;
    std::shared_ptr<Image> unrotatedImage;
    std::shared_ptr<Image> dstImage;
    std::shared_ptr<TileSource> tileSource;
    TileCache tileCache;
    RedrawCallback onRedraw;
    PreRotateCallback onPreRotate;
    int zoomLevel = 0;
    double centerX = 0, centerY = 0;
    bool pendingTiles = true;
    int rotAngle = 0;
    double viewRotation = 0;
    int viewOffsetY = 0;

    bool hasFreeView() const;
    void resizeUnrotatedImage();
    void forEachTileInView(std::function<void(int, int, img::Image &)> f);
};

} /* namespace img */

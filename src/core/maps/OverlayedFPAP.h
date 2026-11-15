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

#include "OverlayedFix.h"

namespace maps {

// Final approach course of an SBAS/GBAS approach (LPV, LP, ...), drawn as a feather
// from the landing threshold (LTP/FTP), or from the FPAP if the threshold is unknown
class OverlayedFPAP : public OverlayedFix {

public:
    OverlayedFPAP(IOverlayHelper *h, const navdb::NavFix *f);

    void configure(const OverlayConfig &cfg, const world::Location &loc) override;
    void drawGraphic() override;
    void drawText(bool detailed) override;

    Hotspot getClickHotspot() const override;

private:
    void setTailCoords();
    void polarToCartesian(float radius, float angleDegrees, double& x, double& y);

    const navdb::FPAP * const navFPAP;

    static constexpr const uint32_t color = img::COLOR_DARK_GREY;
    static constexpr const double FEATHER_LENGTH_NM = 9; // the data has no range for an FPAP, 9nm as shown by Little Navmap

    int textLocationX = 0;
    int textLocationY = 0;

    // Start of the feather
    int startX = 0, startY = 0;

    // End of tail vertices
    int lx, ly; // End of tail, left feather (as viewed from airport)
    int cx, cy; // End of tail, centre indented
    int rx, ry; // End of tail, right feather (as viewed from airport)
};

} /* namespace maps */

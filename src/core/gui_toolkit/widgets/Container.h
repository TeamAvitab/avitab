/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2025 Folke Will
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

#include "Widget.h"

namespace avitab {

class Container: public Widget {
public:
    enum class Fit {
        OFF,
        TIGHT,
        FLOOD,
        FILL
    };

    Container(); // on top layer
    Container(WidgetPtr parent);
    void setLayoutPretty();
    void setLayoutRightColumns();
    void setLayoutRow();
    void setLayoutColumn();
    void setLayoutGrid();
    void setFit(Fit horiz, Fit vert);
private:
    lv_fit_t toLvFit(Fit fit);
};

} /* namespace avitab */

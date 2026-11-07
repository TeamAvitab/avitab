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

#include "XPlaneBaseWindow.h"

class XPlaneVrWindow : public XPlaneBaseWindow {
public:
    XPlaneVrWindow(XPlaneUiSharedData & data);
    ~XPlaneVrWindow() = default;

    void create() override;
    void destroy() override;
    void reposition() override;

protected:
    // these are the callbacks registered for the X-Plane window
    void onDraw();
    XPLMCursorStatus getCursor(int x, int y) { return xplm_CursorDefault; }

    void onWindowMoved(int left, int top, int right, int bottom) override;

private:
    int winCheckTimer;

};

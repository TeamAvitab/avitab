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

#include <XPLM/XPLMDisplay.h>
#include <mutex>

struct XPlaneUiSharedData;

class XPlaneBaseWindow {
    // (mostly?) abstract class for XPlane floating windows - monitor (in-game and popped-out) and VR
public:
    XPlaneBaseWindow(XPlaneUiSharedData & data);
    virtual ~XPlaneBaseWindow() = default;

    // basic X-Plane window management
    virtual void create() = 0;
    virtual void destroy() = 0;
    virtual void reposition() = 0;

    bool isActive();
    void watchdog();

protected:
    void render();
    bool updateWindowGeometry();

    int onLeftClick(int x, int y, XPLMMouseStatus status);
    int onMouseWheel(int x, int y, int wheel, int clicks);
    // called by X-Plane when the pointer moves over the window (no button needed), keeps the
    // pointer position up to date so that the GUI can show hover effects
    XPLMCursorStatus onCursor(int x, int y);

private:
    bool boxelToPixel(int bx, int by, int &px, int &py);

protected:
    XPlaneUiSharedData & common;
    XPLMWindowID winHandle;
    int winWidth, winHeight;
    int noDrawWatchdog;
    // most recently observed window geometry
    int winLeft, winTop, winRight, winBottom;

};

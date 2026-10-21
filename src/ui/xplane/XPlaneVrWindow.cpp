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

#include "XPlaneVrWindow.h"
#include "XPlaneUiDriver.h"
#include "Logger.h"
#include "AviTabBuildSettings.h"
#include <cassert>
#include <cstring>

XPlaneVrWindow::XPlaneVrWindow(XPlaneUiSharedData &data)
:   XPlaneBaseWindow(data),
    winCheckTimer(0)
{
    logger::info("XPlaneVrWindow::XPlaneVrWindow()");
}

void XPlaneVrWindow::create() {
    logger::warn("TESTING XPlaneVrWindow::create()");
    assert(!winHandle);
    if (winHandle) return;

    // Create an XPlane VR (floating) window
    XPLMCreateWindow_t cwp;
    memset(&cwp, 0, sizeof(cwp));
    cwp.structSize = sizeof(cwp);
    cwp.left = 0;
    cwp.top = 0;
    cwp.right = 0;
    cwp.bottom = 0;
    cwp.visible = true;
    cwp.refcon = reinterpret_cast<void*>(this);
    cwp.layer = xplm_WindowLayerFloatingWindows;
    cwp.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
    cwp.drawWindowFunc = [](XPLMWindowID id, void* ref) {
        reinterpret_cast<XPlaneVrWindow*>(ref)->onDraw();
        };
    cwp.handleMouseClickFunc = [](XPLMWindowID id, int x, int y, XPLMMouseStatus status, void* ref) -> int {
        return reinterpret_cast<XPlaneVrWindow*>(ref)->onLeftClick(x, y, status);
        };
    cwp.handleRightClickFunc = nullptr;
    cwp.handleKeyFunc = nullptr;
    cwp.handleCursorFunc = [](XPLMWindowID id, int x, int y, void* ref) -> XPLMCursorStatus {
        return reinterpret_cast<XPlaneVrWindow*>(ref)->getCursor(x, y);
        };
    cwp.handleMouseWheelFunc = [](XPLMWindowID id, int x, int y, int wheel, int clicks, void* ref) -> int {
        return reinterpret_cast<XPlaneVrWindow*>(ref)->onMouseWheel(x, y, wheel, clicks);
        };
    winHandle = XPLMCreateWindowEx(&cwp);
    XPLMSetWindowPositioningMode(winHandle, xplm_WindowVR, -1);
    XPLMSetWindowTitle(winHandle, "Aviator's Tablet " AVITAB_VERSION_STR);
    XPLMSetWindowGeometryVR(winHandle, avitab::UiDriverBase::INITIAL_WIDTH, avitab::UiDriverBase::INITIAL_HEIGHT);
    XPLMSetWindowResizingLimits(winHandle,
        avitab::UiDriverBase::INITIAL_WIDTH, avitab::UiDriverBase::INITIAL_HEIGHT,
        avitab::UiDriverBase::INITIAL_WIDTH, avitab::UiDriverBase::INITIAL_HEIGHT); // essentially disable resizing
    XPLMSetWindowIsVisible(winHandle, 1);
}

void XPlaneVrWindow::destroy() {
    logger::warn("TESTING XPlaneVrWindow::destroy()");
    if (!winHandle) return;

    XPLMDestroyWindow(winHandle);
    winHandle = nullptr;
}

void XPlaneVrWindow::reposition() {
    logger::warn("TESTING XPlaneVrWindow::reposition()");
    if (winHandle) {
        destroy();
    }
    create();
}

void XPlaneVrWindow::onDraw()
{
    assert(winHandle);

    // if we get a draw request then the window must be visible, so cancel the watchdog
    noDrawWatchdog = 0;

    // check the window position and size. don't need to do this every frame, to keep the overheads down
    if (++winCheckTimer > 30) {
        winCheckTimer = 0;
        auto sizeChanged = updateWindowGeometry();
        // TODO - if/when we support change of window size then notify here
    }

    render();
}

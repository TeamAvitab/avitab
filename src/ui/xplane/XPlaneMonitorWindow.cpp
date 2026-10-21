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

#include "XPlaneMonitorWindow.h"
#include "XPlaneUiDriver.h"
#include "Logger.h"
#include "AviTabBuildSettings.h"
#include <cassert>
#include <cstring>

XPlaneMonitorWindow::XPlaneMonitorWindow(XPlaneUiSharedData &data)
:   XPlaneBaseWindow(data),
    winCheckTimer(0)
{
    auto leftTop = defaultCreatePosition();
    winCreateLeft = leftTop.first;
    winCreateTop = leftTop.second;
}

void XPlaneMonitorWindow::create() {
    assert(!winHandle);
    if (winHandle) return;

    int sLeft, sTop, sRight, sBottom;
    (void)screenCentre(sLeft, sTop, sRight, sBottom);

    // sanity check that the desktop window position will have some part of the
    // window title bar visible (and therefore grabbable!)
    winCreateTop = std::min(winCreateTop, sTop - 70);
    winCreateTop = std::max(winCreateTop, sBottom + 20);
    winCreateLeft = std::min(winCreateLeft, sRight - 40);
    auto wr = winCreateLeft + winWidth;
    wr = std::max(wr, sLeft + 40);
    winCreateLeft = wr - winWidth;

    // Create an XPlane desktop (floating) window
    XPLMCreateWindow_t cwp;
    memset(&cwp, 0, sizeof(cwp));
    cwp.structSize = sizeof(cwp);
    cwp.left = winCreateLeft;
    cwp.top = winCreateTop;
    cwp.right = winCreateLeft + winWidth;
    cwp.bottom = winCreateTop - winHeight;
    cwp.visible = true;
    cwp.refcon = reinterpret_cast<void*>(this);
    cwp.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
    cwp.layer = xplm_WindowLayerFloatingWindows;
    cwp.drawWindowFunc = [](XPLMWindowID id, void* ref) {
        reinterpret_cast<XPlaneMonitorWindow*>(ref)->onDraw();
    };
    cwp.handleMouseClickFunc = [](XPLMWindowID id, int x, int y, XPLMMouseStatus status, void* ref) -> int {
        return reinterpret_cast<XPlaneMonitorWindow*>(ref)->onLeftClick(x, y, status);
    };
    cwp.handleRightClickFunc = nullptr;
    cwp.handleKeyFunc = nullptr;
    cwp.handleCursorFunc = [](XPLMWindowID id, int x, int y, void* ref) -> XPLMCursorStatus {
        return reinterpret_cast<XPlaneMonitorWindow*>(ref)->getCursor(x, y);
    };
    cwp.handleMouseWheelFunc = [](XPLMWindowID id, int x, int y, int wheel, int clicks, void* ref) -> int {
        return reinterpret_cast<XPlaneMonitorWindow*>(ref)->onMouseWheel(x, y, wheel, clicks);
    };

    winHandle = XPLMCreateWindowEx(&cwp);
    XPLMSetWindowTitle(winHandle, "Aviator's Tablet " AVITAB_VERSION_STR);
    XPLMSetWindowResizingLimits(winHandle,
        avitab::UiDriverBase::INITIAL_WIDTH, avitab::UiDriverBase::INITIAL_HEIGHT,
        avitab::UiDriverBase::INITIAL_WIDTH, avitab::UiDriverBase::INITIAL_HEIGHT); // essentially disable resizing
    XPLMSetWindowIsVisible(winHandle, 1);
}

void XPlaneMonitorWindow::destroy() {
    if (!winHandle) return;

    winCreateLeft = winLeft;
    winCreateTop = winTop;

    XPLMDestroyWindow(winHandle);
    winHandle = nullptr;
}

void XPlaneMonitorWindow::reposition() {
    if (winHandle) {
        destroy();
    }
    auto leftTop = defaultCreatePosition();
    winCreateLeft = leftTop.first;
    winCreateTop = leftTop.second;
    create();
}

void XPlaneMonitorWindow::onDraw()
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

std::pair<int, int> XPlaneMonitorWindow::screenCentre(int& l, int& t, int& r, int& b)
{
    XPLMGetScreenBoundsGlobal(&l, &t, &r, &b);
    return { (l + r) / 2, (t + b) / 2 };
}

std::pair<int, int> XPlaneMonitorWindow::defaultCreatePosition() {
    int sLeft, sTop, sRight, sBottom;
    auto xy = screenCentre(sLeft, sTop, sRight, sBottom);
    std::pair<int, int> lt;
    lt.first = xy.first - (winWidth / 2);
    lt.second = xy.second + (winHeight / 2);
    return lt;
}

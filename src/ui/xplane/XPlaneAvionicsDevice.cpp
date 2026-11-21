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

#include "XPlaneAvionicsDevice.h"
#include "XPlaneCockpitPanel.h"
#include "XPlaneUiDriver.h"
#include "Logger.h"
#include <XPLM/XPLMGraphics.h>
#include <XPLM/XPLMPlugin.h>
#include <XPLM/XPLMUtilities.h>
#include "platform/OpenGLdefs.h"

XPlaneAvionicsDevice::XPlaneAvionicsDevice(XPlaneUiSharedData & sd, XPlaneCockpitPanel & panel)
:   common(sd),
    cockpitPanel(panel)
{
    int xplaneVersion, xplmVersion;
    XPLMHostApplicationID hostId;
    XPLMGetVersions(&xplaneVersion, &xplmVersion, &hostId);
    if (xplmVersion >= 410) {
        createAvionics = (CreateAvionicsPtr) XPLMFindSymbol("XPLMCreateAvionicsEx");
        destroyAvionics = (DestroyAvionicsPtr) XPLMFindSymbol("XPLMDestroyAvionics");
    }
}

XPlaneAvionicsDevice::~XPlaneAvionicsDevice() {
    destroy();
}

void XPlaneAvionicsDevice::create() {
    if (!createAvionics || !destroyAvionics) {
        logger::info("Avionics devices need X-Plane SDK 4.1.0 or later - device not available");
        return;
    }

    destroy();

    logger::info("Creating avionics device 'teamavitab'");

    static char deviceIdStr[] = "teamavitab";
    static char deviceNameStr[] = "TeamAvitab";

    XPLMCreateAvionics_t params;
    params.structSize = sizeof(params);
    params.screenWidth = common.frameBufferWidth;
    params.screenHeight = common.frameBufferHeight;
    // X-Plane requires the bezel rect to be at least as large as the screen; a
    // bezel equal to the screen size (offset 0,0) means "no border", not size 0.
    params.bezelWidth = common.frameBufferWidth;
    params.bezelHeight = common.frameBufferHeight;
    params.screenOffsetX = 0;
    params.screenOffsetY = 0;
    params.drawOnDemand = 0;
    params.bezelDrawCallback = nullptr;
    params.drawCallback = [] (void *ref) {
        reinterpret_cast<XPlaneAvionicsDevice *>(ref)->onDraw();
    };
    params.bezelClickCallback = nullptr;
    params.bezelRightClickCallback = nullptr;
    params.bezelScrollCallback = nullptr;
    params.bezelCursorCallback = nullptr;
    params.screenTouchCallback = [] (int x, int y, XPLMMouseStatus status, void *ref) -> int {
        return reinterpret_cast<XPlaneAvionicsDevice *>(ref)->onClick(x, y, status);
    };
    params.screenRightTouchCallback = [] (int x, int y, XPLMMouseStatus status, void *ref) -> int {
        return 0;
    };
    params.screenScrollCallback = [] (int x, int y, int wheel, int clicks, void *ref) -> int {
        return reinterpret_cast<XPlaneAvionicsDevice *>(ref)->onWheel(x, y, wheel, clicks);
    };
    params.screenCursorCallback = [] (int x, int y, void *ref) -> XPLMCursorStatus {
        return xplm_CursorDefault;
    };
    params.keyboardCallback = nullptr;
    // Tablet display is always-on: ignore the aircraft's rheostat/ambient/bus voltage
    // Brightness is set by the Brightness Slider
    params.brightnessCallback = [] (float rheo, float ambient, float busVoltsRatio, void *ref) -> float {
        return reinterpret_cast<XPlaneAvionicsDevice *>(ref)->common.brightness;
    };
    params.deviceID = deviceIdStr;
    params.deviceName = deviceNameStr;
    params.refcon = this;

    deviceHandle = createAvionics(&params);
    if (!deviceHandle) {
        // don't throw out of an X-Plane callback; carry on in window-only mode
        logger::error("Couldn't create avionics device - window only mode");
        return;
    }

    cockpitPanel.enableAndPower();
}

void XPlaneAvionicsDevice::destroy() {
    if (deviceHandle && destroyAvionics) {
        logger::info("Removing avionics device");
        destroyAvionics(deviceHandle);
    }
    deviceHandle = nullptr;
}

void XPlaneAvionicsDevice::onDraw() {
    int w = common.frameBufferWidth;
    int h = common.frameBufferHeight;

    // The device's screen is a persistent framebuffer that X-Plane does not clear between
    // calls, so an explicitly blanked screen is drawn whenever AviTab isn't showing content.
    if (!cockpitPanel.isEnabled() || !cockpitPanel.isPowered() || common.glTextureId < 0) {
        XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

    XPLMBindTexture2d(common.glTextureId, 0);
    common.redrawTexture();

    XPLMSetGraphicsState(0, 1, 0, 0, 0, 0, 0);
    glColor3f(1, 1, 1);
    renderTexture(0, h, w, 0);
}

void XPlaneAvionicsDevice::renderTexture(int left, int top, int right, int bottom) {
    // our window has a negative y-axis while OpenGL has a positive one
    glBegin(GL_QUADS);
        glTexCoord2i(0, 1);
        glVertex2i(left, bottom);
        glTexCoord2i(0, 0);
        glVertex2i(left, top);
        glTexCoord2i(1, 0);
        glVertex2i(right, top);
        glTexCoord2i(1, 1);
        glVertex2i(right, bottom);
    glEnd();
}

bool XPlaneAvionicsDevice::onClick(int x, int y, XPLMMouseStatus status) {
    if (!cockpitPanel.isEnabled()) {
        return false;
    }

    switch (status) {
    case xplm_MouseDown:
    case xplm_MouseDrag:
        common.mousePressed = true;
        break;
    case xplm_MouseUp:
        common.mousePressed = false;
        break;
    default:
        return false;
    }

    // the device reports screen coordinates with y=0 at the bottom, but AviTab's own
    // buffer coordinates have y=0 at the top
    common.mouseX = x;
    common.mouseY = common.frameBufferHeight - 1 - y;

    return true;
}

bool XPlaneAvionicsDevice::onWheel(int x, int y, int wheel, int clicks) {
    if (!cockpitPanel.isEnabled()) {
        return false;
    }

    common.mouseX = x;
    common.mouseY = common.frameBufferHeight - 1 - y;
    common.wheelClicks = clicks;

    return true;
}

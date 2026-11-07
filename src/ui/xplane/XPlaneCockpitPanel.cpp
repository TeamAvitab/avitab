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

#include "XPlaneCockpitPanel.h"
#include "XPlaneUiDriver.h"
#include "JsonConfig.h"
#include "Logger.h"
#include <XPLM/XPLMGraphics.h>
#include <XPLM/XPLMUtilities.h>
#include "platform/OpenGLdefs.h"
#include <cassert>
#include <cstring>

XPlaneCockpitPanel::XPlaneCockpitPanel(XPlaneUiSharedData & sd)
:   common(sd),
    panelPowered(false), panelEnabled(false),
    panelLeft(0), panelBottom(0), panelWidth(0), panelHeight(0),
    panelClickX(std::numeric_limits<float>::quiet_NaN()),
    panelClickY(std::numeric_limits<float>::quiet_NaN()),
    xplane3dClickX("sim/graphics/view/click_3d_x_pixels", -1),
    xplane3dClickY("sim/graphics/view/click_3d_y_pixels", -1),
    mouseClickCaptured(false),
    buttonRef(XPLMFindDataRef("sim/joystick/joystick_button_values"))
{
    panelPoweredRef = std::make_unique<xdata::DataRefExport<int>>("avitab/panel_powered", this,
        [] (void *self) { return (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelPowered; },
        [] (void *self, int v) { (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelPowered = v; });

    panelEnabledRef = std::make_unique<xdata::DataRefExport<int>>("avitab/panel_enabled", this,
        [] (void *self) { return (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelEnabled; },
        [] (void *self, int v) { (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelEnabled = v; });

    panelLeftRef = std::make_unique<xdata::DataRefExport<int>>("avitab/panel_left", this,
        [] (void *self) { return (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelLeft; },
        [] (void *self, int v) { (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelLeft = v; });

    panelWidthRef = std::make_unique<xdata::DataRefExport<int>>("avitab/panel_width", this,
        [] (void *self) { return (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelWidth; },
        [] (void *self, int v) { (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelWidth = v; });

    panelBottomRef = std::make_unique<xdata::DataRefExport<int>>("avitab/panel_bottom", this,
        [] (void *self) { return (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelBottom; },
        [] (void *self, int v) { (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelBottom = v; });

    panelHeightRef = std::make_unique<xdata::DataRefExport<int>>("avitab/panel_height", this,
        [] (void *self) { return (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelHeight; },
        [] (void *self, int v) { (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelHeight = v; });

    panelMouseXref = std::make_unique<xdata::DataRefExport<float>>("avitab/panel_x_click", this,
        [] (void *self) { return (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelClickX; },
        [] (void *self, float x) { (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelClickX = x; });

    panelMouseYref = std::make_unique<xdata::DataRefExport<float>>("avitab/panel_y_click", this,
        [] (void *self) { return (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelClickY; },
        [] (void *self, float y) { (reinterpret_cast<XPlaneCockpitPanel *>(self))->panelClickY = y; });
}

XPlaneCockpitPanel::~XPlaneCockpitPanel() {
    panelEnabledRef.reset();
    panelPoweredRef.reset();
    panelLeftRef.reset();
    panelWidthRef.reset();
    panelBottomRef.reset();
    panelHeightRef.reset();
    panelMouseXref.reset();
    panelMouseYref.reset();
}

void XPlaneCockpitPanel::onPlaneLoad(const std::filesystem::path &aircraftDir) {

    // first remove existing panel (if any)
    destroy();

    // now look for AviTab.json in the new aircraftDir, and take it from there!
    auto cfgFile = aircraftDir / "AviTab.json";
    try {
        avitab::JsonConfig cfg(cfgFile);
        int left = cfg.getInt("/panel/left");
        int bottom = cfg.getInt("/panel/bottom");
        int width = cfg.getInt("/panel/width");
        int height = cfg.getInt("/panel/height");
        bool enable = false;
        bool disableCaptureWindow = false;
        bool aircraftManaged = false;
        hideHeaderApp = false;
        try {
            enable = cfg.getBool("/panel/enabled");
        } catch (...) {
        }
        try {
            hideHeaderApp = cfg.getBool("/panel/hide_header");
        } catch (...) {
        }
        try {
            disableCaptureWindow = cfg.getBool("/panel/disable_capture_window");
        } catch (...) {
        }
        try {
            aircraftManaged = cfg.getBool("/panel/aircraft_managed");
        } catch (...) {
        }

        panelMode = aircraftManaged ? PanelMode::AIRCRAFT_MANAGED
                                    : (disableCaptureWindow ? PanelMode::COMMAND_ONLY
                                                            : PanelMode::CAPTURE_WINDOW);

        create(left, bottom, width, height, panelMode, enable);
    } catch (const std::exception &e) {
        logger::info("No panel config - window only mode");
        destroy();
    }
}

void XPlaneCockpitPanel::onVRswitch(bool inVR) {
    vrTriggerIndices.clear();
    if (inVR) {
        setupVRCapture();
    }
}

void XPlaneCockpitPanel::create(int left, int bottom, int width, int height, PanelMode mode, bool enable) {
    logger::info("Creating cockpit panel @ %d,%d size %dx%d mode=%s",
                left,bottom, width,height,
                mode == PanelMode::CAPTURE_WINDOW ? "capture" : (mode == PanelMode::COMMAND_ONLY ? "hybrid" : "managed"));

    assert(!captureWindowHandle);
    
    panelLeft = left;
    panelBottom = bottom;
    panelWidth = width;
    panelHeight = height;
    if (enable) {
        panelEnabled = true;
        panelPowered = true;
    }

    if (mode == PanelMode::CAPTURE_WINDOW) {
        createCaptureWindow();
    }

    XPLMRegisterDrawCallback(onDrawCallback, xplm_Phase_Gauges, false, this);
}

void XPlaneCockpitPanel::destroy() {
    logger::info("Destroying cockpit panel");

    if (captureWindowHandle) {
        XPLMDestroyWindow(captureWindowHandle);
        captureWindowHandle = nullptr;
    }

    XPLMUnregisterDrawCallback(onDrawCallback, xplm_Phase_Gauges, false, this);

    panelMode = PanelMode::NO_PANEL;
    hideHeaderApp = false;
}

int XPlaneCockpitPanel::onDrawCallback(XPLMDrawingPhase phase, int isBefore, void *ref) {
    XPlaneCockpitPanel *us = reinterpret_cast<XPlaneCockpitPanel *>(ref);
    us->drawPanel();
    return 1;
}

void XPlaneCockpitPanel::drawPanel() {
    if (!panelEnabled) return;

    int left = panelLeft;
    int top = panelBottom + panelHeight;
    int right = panelLeft + panelWidth;
    int bottom = panelBottom;

    common.correctRatio(left, top, right, bottom, true);

    if (panelPowered == 0) {
        XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0);
        glColor3f(0, 0, 0);
        glBegin(GL_QUADS);
            glVertex2i(left, bottom);
            glVertex2i(left, top);
            glVertex2i(right, top);
            glVertex2i(right, bottom);
        glEnd();
        return;
    }

    XPLMBindTexture2d(common.glTextureId, 0);
    common.redrawTexture();

    XPLMSetGraphicsState(0, 1, 0, 0, 0, 0, 0);
    float b = common.brightness;
    glColor3f(b, b, b);
    renderTexture(left, top, right, bottom);

    if (vrTriggerIndices.size() && (panelMode == PanelMode::CAPTURE_WINDOW)) {
        // Capture window mouse click callback does not work in VR mode.
        // But we can sniff the VR controller trigger and use this instead
        sniffVRtrigger();
    }
}

void XPlaneCockpitPanel::renderTexture(int left, int top, int right, int bottom) {
    // our window has a negative y-axis while OpenGL has a positive one
    glBegin(GL_QUADS);
        // map top left texture to bottom left vertex
        glTexCoord2i(0, 1);
        glVertex2i(left, bottom);

        // map bottom left texture to top left vertex
        glTexCoord2i(0, 0);
        glVertex2i(left, top);

        // map bottom right texture to top right vertex
        glTexCoord2i(1, 0);
        glVertex2i(right, top);

        // map top right texture to bottom right vertex
        glTexCoord2i(1, 1);
        glVertex2i(right, bottom);
    glEnd();
}

void XPlaneCockpitPanel::createCaptureWindow() {
    int winLeft, winTop, winRight, winBot;
    XPLMGetScreenBoundsGlobal(&winLeft, &winTop, &winRight, &winBot);

    XPLMCreateWindow_t cwp;
    memset(&cwp, 0, sizeof(cwp));
    cwp.structSize = sizeof(cwp);
    cwp.left = winLeft;
    cwp.right = winRight;
    cwp.top = winTop;
    cwp.bottom = winBot;
    cwp.visible = 1;
    cwp.refcon = this;
    cwp.drawWindowFunc = [] (XPLMWindowID id, void *ref) {
    };
    cwp.handleMouseClickFunc = [] (XPLMWindowID id, int x, int y, XPLMMouseStatus status, void *ref) -> int {
        bool pressed = (status != xplm_MouseUp);
        return reinterpret_cast<XPlaneCockpitPanel *>(ref)->onPanelClick(pressed);
    };
    cwp.handleCursorFunc = [] (XPLMWindowID id, int x, int y, void *ref) -> XPLMCursorStatus {
        return xplm_CursorDefault;
    };
    cwp.handleMouseWheelFunc =  [] (XPLMWindowID id, int x, int y, int wheel, int clicks, void *ref) -> int {
        if (wheel != 0) return false;
        return reinterpret_cast<XPlaneCockpitPanel *>(ref)->onPanelWheel(clicks);
    };
    cwp.handleRightClickFunc = nullptr;
    cwp.handleKeyFunc = nullptr;
    cwp.layer = xplm_WindowLayerFlightOverlay;
    cwp.decorateAsFloatingWindow = xplm_WindowDecorationNone;
    captureWindowHandle = XPLMCreateWindowEx(&cwp);
    XPLMSetWindowPositioningMode(captureWindowHandle, xplm_WindowFullScreenOnAllMonitors, -1);
}

void XPlaneCockpitPanel::setupVRCapture() {
    if (!buttonRef) {
        logger::warn("Could not setup VR trigger check: button values ref not found");
        return;
    }

    int triggerIndex = (ptrdiff_t) XPLMFindCommand("sim/VR/reserved/select");
    if (triggerIndex == 0) {
        logger::warn("Could not setup VR trigger check: command not found");
        return;
    }

    auto assignmentsRef = XPLMFindDataRef("sim/joystick/joystick_button_assignments");
    if (!assignmentsRef) {
        logger::warn("Could not setup VR trigger check: assignments ref not found");
        return;
    }

    int assignments[3200];
    XPLMGetDatavi(assignmentsRef, assignments, 0, 3200);
    for (int i = 0; i < 3200; i++) {
        if (assignments[i] == triggerIndex) {
            vrTriggerIndices.push_back(i);
        }
    }

    if (vrTriggerIndices.empty()) {
        logger::warn("Could not setup VR trigger check: trigger assignment not found");
        return;
    }
}

void XPlaneCockpitPanel::sniffVRtrigger() {
    bool gotAnyTrigger = false;
    for (auto idx: vrTriggerIndices) {
        int triggerVal = 0;
        XPLMGetDatavi(buttonRef, &triggerVal, idx, 1);
        if (triggerVal) {
            onPanelClick(true);
            triggerClickSniffed = true;
            gotAnyTrigger = true;
        }
    }
    if (!gotAnyTrigger && triggerClickSniffed) {
        onPanelClick(false);
        triggerClickSniffed = false;
    }
}

bool XPlaneCockpitPanel::onPanelClick(bool pressed) {
    auto active = panelEnabled && panelPowered && (panelMode != PanelMode::NO_PANEL);
    if (!active) return false;

    float px, py;
    int mx, my;
    bool isInWindow = panelClickXYtoAvitabXY(px, py, mx, my);
    common.mouseX = mx;
    common.mouseY = my;

    if (pressed) {
        if (isInWindow && !mouseClickCaptured) {
            // first mouse button down report, and in the AviTab panel
            common.mousePressed = true;
        }
        mouseClickCaptured = true;
    } else {
        // mouse released
        common.mousePressed = false;
        mouseClickCaptured = false;
    }

    return isInWindow || common.mousePressed;
}

bool XPlaneCockpitPanel::onPanelWheel(int clicks) {
    auto active = panelEnabled && panelPowered && (panelMode != PanelMode::NO_PANEL);
    if (!active) return false;

    float px, py;
    int mx, my;
    bool isInWindow = panelClickXYtoAvitabXY(px, py, mx, my);
    common.mouseX = mx;
    common.mouseY = my;

    if (isInWindow) {
        common.wheelClicks = clicks;
        return true;
    }
    return false;
}

bool XPlaneCockpitPanel::panelClickXYtoAvitabXY(float & px, float & py, int & mx, int & my) {
    int left = panelLeft;
    int top = panelBottom + panelHeight;
    int right = panelLeft + panelWidth;
    int bottom = panelBottom;
    common.correctRatio(left, top, right, bottom, true);

    if (panelMode == PanelMode::AIRCRAFT_MANAGED) {
        // use the avitab mouse position datatrefs in aircraft managed mode
        px = panelClickX;
        py = panelClickY;
    } else {
        // use the mouse location reported X-Plane's built-in datarefs in the older modes
        px = xplane3dClickX;
        py = xplane3dClickY;
    }

    mx = (px - left) / (right - left) * common.frameBufferWidth;
    my = (top - py) / (top - bottom) * common.frameBufferHeight;
    bool isInAvitabWindow = (mx >= 0) && (mx < common.frameBufferWidth) && (my >= 0) && (my < common.frameBufferHeight);

    return isInAvitabWindow;
}



#if 0 // this is stuff that I think is panel-specific

void XPlaneUiDriver::createPanel(int left, int bottom, int width, int height, PanelControlMode mode) {
    logger::info("Creating panel @ %d,%d size %dx%d mode=%s",
        left,bottom, width,height,
        mode == PanelControlMode::CAPTURE_WINDOW ? "capture" : (mode == PanelControlMode::COMMAND_ONLY ? "hybrid" : "managed"));
    if (captureWindow) {
        XPLMDestroyWindow(captureWindow);
        captureWindow = {};
    }

    if (mode == PanelControlMode::CAPTURE_WINDOW) {
    }

    panelLeft = left;
    panelBottom = bottom;
    panelWidth = width;
    panelHeight = height;

    XPLMRegisterDrawCallback(onDraw3D, xplm_Phase_Gauges, false, this);
    panelClickX = std::numeric_limits<float>::quiet_NaN();
    panelClickY = std::numeric_limits<float>::quiet_NaN();
    panelControlMode = mode;
    isPanelActive = true;
}

void XPlaneUiDriver::hidePanel() {
    logger::info("Removing/hiding panel");
    if (captureWindow) {
        XPLMDestroyWindow(captureWindow);
        captureWindow = {};
    }
    XPLMUnregisterDrawCallback(onDraw3D, xplm_Phase_Gauges, false, this);
    isPanelActive = false;
}





#endif

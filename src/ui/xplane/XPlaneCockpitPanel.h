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

#include "sim/xplane/DataRefExport.h"
#include "sim/xplane/DataRefImport.h"
#include <XPLM/XPLMDisplay.h>
#include <filesystem>
#include <memory>

struct XPlaneUiSharedData;

class XPlaneCockpitPanel {

public:
    XPlaneCockpitPanel(XPlaneUiSharedData & data);
    ~XPlaneCockpitPanel();

    void onPlaneLoad(const std::filesystem::path & aircraftDir);
    void onVRswitch(bool inVR);

    void leftClickFeed(bool pressed) { (void)onPanelClick(pressed); }
    void mouseWheelFeed(int dir) { (void)onPanelWheel(dir); }

    bool isActive() const { return (panelMode != PanelMode::NO_PANEL); }
    bool isHeaderAppHidden() const { return hideHeaderApp; }

private:
    enum class PanelMode {
        NO_PANEL,
        CAPTURE_WINDOW,
        COMMAND_ONLY,
        AIRCRAFT_MANAGED
    };

private:
    void create(int left, int bottom, int width, int height, PanelMode mode, bool enable);
    void destroy();
    static int onDrawCallback(XPLMDrawingPhase phase, int isBefore, void *ref);
    void drawPanel();
    void renderTexture(int left, int top, int right, int bottom);

    void createCaptureWindow();
    void setupVRCapture();
    void sniffVRtrigger();

    bool onPanelClick(bool pressed);
    bool onPanelWheel(int clicks);

    bool panelClickXYtoAvitabXY(float & px, float & py, int & mx, int & my);

private:
    XPlaneUiSharedData & common;

    // Some of the panel state is exported as X-Plane data refs which can be read/written by
    // the integrating aircraft. Each has a corresponding native variable.
    std::unique_ptr<xdata::DataRefExport<int>> panelPoweredRef, panelEnabledRef;
    int panelPowered, panelEnabled;
    std::unique_ptr<xdata::DataRefExport<int>> panelLeftRef, panelBottomRef, panelWidthRef, panelHeightRef;
    int panelLeft, panelBottom, panelWidth, panelHeight;
    std::unique_ptr<xdata::DataRefExport<float>> panelMouseXref, panelMouseYref;
    float panelClickX, panelClickY;

    // These are X-Plane datarefs that indicate the mouse location in the 3D cockpit. Only
    // functional in XP11 and some older XP12 integrations.
    xdata::DataRefImport<float> xplane3dClickX, xplane3dClickY;

    // Current panel mode, none, or whatever the aircraft config is set to.
    PanelMode panelMode = PanelMode::NO_PANEL;
    bool hideHeaderApp = false;

    // A transparent window used to detect mouse clicks in CAPTURE_WINDOW mode.
    XPLMWindowID captureWindowHandle = nullptr;

    // Indicates that a capture window mouse down callback is currently active
    bool mouseClickCaptured;

    // Indicates that a VR controller trigger click was detected when sniffing.
    bool triggerClickSniffed;

    // Related to detection of the VR pointer over the panel (mouse non-functional in VR w/capture window)
    std::vector<int> vrTriggerIndices;
    XPLMDataRef buttonRef;

};

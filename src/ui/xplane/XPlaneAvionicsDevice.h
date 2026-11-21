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

struct XPlaneUiSharedData;
class XPlaneCockpitPanel;

// A self-contained X-Plane 12 avionics device ("teamavitab") that is used for aircraft
// which don't ship an AviTab.json, so that aircraft can reference AviTab via
// ATTR_cockpit_device "teamavitab" without any further config.
// The device shares the avitab/panel_enabled and avitab/panel_powered datarefs
// with the (legacy) cockpit panel.
class XPlaneAvionicsDevice {

public:
    XPlaneAvionicsDevice(XPlaneUiSharedData & data, XPlaneCockpitPanel & panel);
    ~XPlaneAvionicsDevice();

    void create();
    void destroy();

    bool isActive() const { return deviceHandle != nullptr; }

private:
    void onDraw();
    bool onClick(int x, int y, XPLMMouseStatus status);
    bool onWheel(int x, int y, int wheel, int clicks);
    void renderTexture(int left, int top, int right, int bottom);

private:
    XPlaneUiSharedData & common;
    XPlaneCockpitPanel & cockpitPanel;

    // XPLMCreateAvionicsEx needs XPLM410 (X-Plane 12.1+); it is looked up at runtime so
    // the plugin still loads and works on older versions.
    using CreateAvionicsPtr = XPLMAvionicsID(*)(XPLMCreateAvionics_t *);
    using DestroyAvionicsPtr = void(*)(XPLMAvionicsID);
    CreateAvionicsPtr createAvionics = nullptr;
    DestroyAvionicsPtr destroyAvionics = nullptr;

    XPLMAvionicsID deviceHandle = nullptr;
};

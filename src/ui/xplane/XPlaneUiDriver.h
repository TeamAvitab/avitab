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

#include "UiDriver.h"
#include "sim/xplane/DataRefImport.h"
#include "sim/xplane/DataRefExport.h"
#include "XPlaneMonitorWindow.h"
#include "XPlaneVrWindow.h"
#include "XPlaneCockpitPanel.h"
#include <atomic>
#include <filesystem>
#include <mutex>
#include <memory>

class XPlaneSimDriver;

struct XPlaneUiSharedData {
    // This structure is a convenient way of sharing various state data and
    // other attributes between the classes that represent the different types of windows,
    // panels (and eventually avionics). Separating the behaviours of windows, panels and
    // avionics makes the code easier to follow, but since they all share quite a lot of
    // data with each other, or with the top-level UI driver, collecting it here seems the
    // easiest way to handle that (without adding tons of accessor functions).

    // This is the ID that identifies the GL texture which gets painted with the latest
    // Avitab screen image whenever that gets changed and notified through 'blit'. Both
    // windows and panel (and avionics) will use this for their GL rendering.
    int glTextureId = -1;
    bool textureNeedsRedraw = false;
    std::mutex textureRedrawMutex;
    void notifyRedraw();
    void redrawTexture();

    // Stuff related to the framebuffer (which gets drawn to the texture buffer)
    const uint32_t *frameBufferData;
    int frameBufferWidth, frameBufferHeight;
    void correctRatio(int &left, int &top, int &right, int &bottom, bool center);

    // This is the current brightness setting, which is also controlled by an X-Plane dataref.
    std::unique_ptr<xdata::DataRefExport<float>> brightnessRef;
    float brightness = 1.0;

    // These members reflect the current position of the mouse, the state of the button
    // and any pending wheel rotations (+ve = up, -ve = down)
    std::atomic_int mouseX = 0, mouseY = 0;
    std::atomic_bool mousePressed = false;
    std::atomic_int wheelClicks = 0;
};

class XPlaneUiDriver: public avitab::UiDriverBase {
public:
    XPlaneUiDriver();
    ~XPlaneUiDriver();

public:
    // The top-level state machine is start -> enable -> disable { -> enable -> disable } -> stop
    void start(std::shared_ptr<avitab::Settings> settings, std::shared_ptr<avitab::AviTabCore> core, std::shared_ptr<XPlaneSimDriver> sim);
    void enable(const std::filesystem::path & aircraftDir);
    void disable();
    void stop();

    void toggleTablet();
    void resetWindowPosition();
    void onPlaneLoad(const std::filesystem::path & aircraftDir);
    void onVRswitch();

    bool isHeaderAppHidden();
    
    void watchdog();

public: // Implementation of UiDriverBase
    void requestCloseWindow() override;

    void blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t *data) override;

    void leftClickFeed(bool pressed);
    void mouseWheelFeed(int dir);

    void readPointerState(int &x, int &y, bool &pressed) override;
    int getWheelClicks() override;

    void setBrightness(float b) override;
    float getBrightness() override;

private:
    static int onKeySniff(char c, XPLMKeyFlags flags, char vKey, void *ref);

private:
    std::shared_ptr<avitab::Settings> settings;

    xdata::DataRefImport<bool> isVrEnabled;

    XPlaneUiSharedData sharedData;

    std::unique_ptr<XPlaneMonitorWindow> monitorWindow;
    std::unique_ptr<XPlaneVrWindow> vrWindow;
    XPlaneBaseWindow * currentWindow;
    std::unique_ptr<XPlaneCockpitPanel> cockpitPanel;

    bool appCloseRequested;

};

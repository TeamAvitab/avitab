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

#include "XPlaneUiDriver.h"
#include "Logger.h"
#include <XPLM/XPLMGraphics.h>
#include "platform/OpenGLdefs.h"


XPlaneUiDriver::XPlaneUiDriver()
:   isVrEnabled("sim/graphics/VR/enabled", false),
    appCloseRequested(false)
{
}

XPlaneUiDriver::~XPlaneUiDriver() {
}

void XPlaneUiDriver::start(std::shared_ptr<avitab::Settings> prefs, std::shared_ptr<avitab::AviTabCore> core, std::shared_ptr<XPlaneSimDriver> sim) {
    sharedData.brightnessRef = std::make_unique<xdata::DataRefExport<float>>("avitab/brightness", this,
        [] (void *self) { return (reinterpret_cast<XPlaneUiDriver *>(self))->sharedData.brightness; },
        [] (void *self, float v) { (reinterpret_cast<XPlaneUiDriver *>(self))->sharedData.brightness = v; });
    sharedData.frameBufferData = data();
    sharedData.frameBufferWidth = width();
    sharedData.frameBufferHeight = height();

    // create the floating window and cockpit panel owning objects
    monitorWindow = std::make_unique<XPlaneMonitorWindow>(sharedData);
    vrWindow = std::make_unique<XPlaneVrWindow>(sharedData);
    cockpitPanel = std::make_unique<XPlaneCockpitPanel>(sharedData);
}

void XPlaneUiDriver::enable(const std::filesystem::path & aircraftDir) {
    logger::info("XPlaneUiDriver::enable()");

#if 0 // REFACTOR-TODO - avitab equivalent required   
    auto& xwdp = prefs->Get("/xplane/window");
    try {
        showNow = xwdp.at("/open_at_start"_json_pointer);
    }
    catch (...) {}
    LOGD(fmt::format("Read open_at_start preference as {}", showNow));
#endif

    currentWindow = isVrEnabled ? static_cast<XPlaneBaseWindow *>(vrWindow.get()) : static_cast<XPlaneBaseWindow *>(monitorWindow.get());
    cockpitPanel->onPlaneLoad(aircraftDir);

    // Initialise the GL texture that we'll render the Avitab screen into so X-Plane can paint it
    XPLMGenerateTextureNumbers(&sharedData.glTextureId, 1);
    XPLMBindTexture2d(sharedData.glTextureId, 0);
    glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, this->width(), this->height(), 0,
            GL_BGRA, GL_UNSIGNED_BYTE, data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // We can't really use the window manager handleKeyFunc, since our windows
    // don't use the X-Plane UI widgets for text entry. So we use a key sniffer
    // which only consumes key events when the GUI can use them.
    XPLMRegisterKeySniffer(onKeySniff, 0, this);
}

void XPlaneUiDriver::disable() {
    logger::info("XPlaneUiDriver::disable()");

    XPLMUnregisterKeySniffer(onKeySniff, 0, this);

#if 0 // REFACTOR-TODO - avitab equivalent required   
    // save the window state for next time
    auto xwdp = prefs->Get("/xplane/window");
    xwdp["open_at_start"] = panel->isActive();
    prefs->Put("/xplane/window", xwdp);
    LOGD(fmt::format("Wrote open_at_start preference to {}", panel->isActive()));
#endif

    // hide and destroy the floating window
    currentWindow->destroy();
    currentWindow = nullptr;
    // disable the cockpit panel
    cockpitPanel->onPlaneLoad("");

    GLuint gluId = sharedData.glTextureId;
    glDeleteTextures(1, &gluId);
    sharedData.glTextureId = -1;
}

void XPlaneUiDriver::stop() {
    monitorWindow.reset();
    vrWindow.reset();
    cockpitPanel.reset();
    sharedData.brightnessRef.reset();
}

void XPlaneUiDriver::watchdog() {
    if (currentWindow) {
        currentWindow->watchdog();
        if (appCloseRequested) {
            currentWindow->destroy();
        }
    }
}

void XPlaneUiDriver::toggleTablet() {
    if (!currentWindow) return;
    if (currentWindow->isActive()) {
        currentWindow->destroy();
    } else {
        appCloseRequested = false;
        currentWindow->create();
    }
}

void XPlaneUiDriver::resetWindowPosition() {
    currentWindow->reposition();
}

void XPlaneUiDriver::onPlaneLoad(const std::filesystem::path & aircraftDir) {
    if (isVrEnabled && currentWindow->isActive()) {
        // in VR mode the window is re-opened to force repositioning in the new 3D arena
        currentWindow->destroy();
        currentWindow->create();
    }
    // notify the cockpit panel that the plane has changed
    cockpitPanel->onPlaneLoad(aircraftDir);
}

void XPlaneUiDriver::onVRswitch() {
    logger::info("VR mode switch notified: now %d", isVrEnabled);
    auto active = currentWindow->isActive();
    currentWindow->destroy();
    currentWindow = isVrEnabled ? static_cast<XPlaneBaseWindow *>(vrWindow.get()) : static_cast<XPlaneBaseWindow *>(monitorWindow.get());
    if (active) currentWindow->create();
    cockpitPanel->onVRswitch(isVrEnabled);
}

bool XPlaneUiDriver::isHeaderAppHidden() {
    return cockpitPanel->isHeaderAppHidden();
}

void XPlaneUiDriver::requestCloseWindow() {
    appCloseRequested = true;
}

void XPlaneUiDriver::blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t* data) {
    UiDriverBase::blit(x1, y1, x2, y2, data);
    sharedData.notifyRedraw();
}

void XPlaneUiDriver::leftClickFeed(bool pressed) {
    cockpitPanel->leftClickFeed(pressed);
}

void XPlaneUiDriver::mouseWheelFeed(int dir) {
    cockpitPanel->mouseWheelFeed(dir);
}

void XPlaneUiDriver::readPointerState(int &x, int &y, bool &pressed) {
    x = sharedData.mouseX;
    y = sharedData.mouseY;
    pressed = sharedData.mousePressed;
}

int XPlaneUiDriver::getWheelClicks() {
    int val = sharedData.wheelClicks;
    sharedData.wheelClicks = 0;
    return val;
}

int XPlaneUiDriver::onKeySniff(char c, XPLMKeyFlags flags, char vKey, void* ref) {
    XPlaneUiDriver *us = (XPlaneUiDriver *) ref;

    // ignore ctrl and other modifier keys
    if ((flags & xplm_OptionAltFlag) || (flags & xplm_ControlFlag)) {
        return 1;
    }

    // only do anything if the GUI currently wants keys, and there is an active display (window or panel)
    if (us->wantsKeyInput() && (us->currentWindow->isActive() || us->cockpitPanel->isActive())) {
        if (flags & xplm_UpFlag) {
            return 0;
        }

        if (c == '\r') {
            c = '\n';
        }

        us->pushKeyInput(c);
        return 0;
    }
    return 1;
}

void XPlaneUiDriver::setBrightness(float b) {
    sharedData.brightness = b;
}

float XPlaneUiDriver::getBrightness() {
    return sharedData.brightness;
}



void XPlaneUiSharedData::correctRatio(int &left, int &top, int& right, int& bottom, bool center) {
    int curWidth = right - left;
    int curHeight = top - bottom;

    float ourRatio = (float) frameBufferHeight / frameBufferWidth;

    if (curWidth * ourRatio <= curHeight) {
        int newHeight = curWidth * ourRatio;
        if (center) {
            bottom += (curHeight - newHeight) / 2;
            top = bottom + newHeight;
        } else {
            bottom = top - newHeight;
        }
    } else {
        int newWidth = curHeight / ourRatio;
        if (center) {
            left += (curWidth - newWidth) / 2;
            right = left + newWidth;
        } else {
            right = left + newWidth;
        }
    }
}

void XPlaneUiSharedData::notifyRedraw() {
    std::lock_guard<std::mutex> lock(textureRedrawMutex);
    textureNeedsRedraw = true;
}

void XPlaneUiSharedData::redrawTexture() {
    std::lock_guard<std::mutex> lock(textureRedrawMutex);
    if (textureNeedsRedraw) {
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                0, 0,
                frameBufferWidth, frameBufferHeight,
                GL_BGRA, GL_UNSIGNED_BYTE, frameBufferData);
        textureNeedsRedraw = false;
    }
}












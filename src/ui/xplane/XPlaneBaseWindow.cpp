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

#include "XPlaneBaseWindow.h"
#include "XPlaneUiDriver.h"
#include <XPLM/XPLMGraphics.h>
#include "platform/OpenGLdefs.h"

XPlaneBaseWindow::XPlaneBaseWindow(XPlaneUiSharedData &data)
:   common(data),
    winHandle(nullptr),
    winWidth(avitab::UiDriverBase::INITIAL_WIDTH),
    winHeight(avitab::UiDriverBase::INITIAL_HEIGHT),
    noDrawWatchdog(0),
    winLeft(0), winTop(0), winRight(0), winBottom(0)
{
}

void XPlaneBaseWindow::render() {
    XPLMBindTexture2d(common.glTextureId, 0);
    common.redrawTexture();

    XPLMSetGraphicsState(0, 1, 0, 0, 0, 0, 0);

    float b = common.brightness;
    glColor3f(b, b, b);

    // our window has a negative y-axis while OpenGL has a positive one
    int left, top, right, bottom;
    XPLMGetWindowGeometry(winHandle, &left, &top, &right, &bottom);
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

bool XPlaneBaseWindow::updateWindowGeometry() {
    int l, t, r, b;
    XPLMGetWindowGeometry(winHandle, &l, &t, &r, &b);
    if ((winLeft != l) || (winTop != t) || (winRight != r) || (winBottom != b)) {
        onWindowMoved(l, t, r, b);
#if 0 // TODO - we don't need this until window resize is supported
        int w = r - l;
        int h = t - b;
        if ((w != winWidth) || (h != winHeight)) {
            winWidth = w; winHeight = h;
            return true;
        }
#endif
    }
    return false;
}

int XPlaneBaseWindow::onLeftClick(int x, int y, XPLMMouseStatus status)
{
    int guiX, guiY;

    bool isInWindow = boxelToPixel(x, y, guiX, guiY);

    switch (status) {
    case xplm_MouseDown:
        if (isInWindow) {
            common.mousePressed = true;
        }
        break;
    case xplm_MouseDrag:
        common.mousePressed = true;
        break;
    case xplm_MouseUp:
        common.mousePressed = false;
        break;
    default:
        isInWindow = false;
    }

    if (isInWindow) {
        common.mouseX = guiX;
        common.mouseY = guiY;
    }

    return 1;
}

int XPlaneBaseWindow::onMouseWheel(int x, int y, int wheel, int clicks)
{
    int guiX, guiY;
    if (boxelToPixel(x, y, guiX, guiY)) {
        common.mouseX = guiX;
        common.mouseY = guiY;
        common.wheelClicks = clicks;
        return 1;
    }
    return 0;
}

bool XPlaneBaseWindow::boxelToPixel(int bx, int by, int &px, int &py) {
    int bLeft, bTop, bRight, bBottom;
    XPLMGetWindowGeometry(winHandle, &bLeft, &bTop, &bRight, &bBottom);

    common.correctRatio(bLeft, bTop, bRight, bBottom, false);

    if (bLeft == bRight || bTop == bBottom) {
        px = -1;
        py = -1;
        return false;
    }

    // calculate the center of the window in boxels
    int bCenterX = bLeft + (bRight - bLeft) / 2;
    int bCenterY = bBottom + (bTop - bBottom) / 2;

    // normalized vector from center to point
    float vecX = (bx - bCenterX) / float(bRight - bLeft);
    float vecY = (by - bCenterY) / float(bTop - bBottom);

    // GUI center in pixels
    int guiWidth = common.frameBufferWidth;
    int guiHeight = common.frameBufferHeight;
    int pCenterX = guiWidth / 2;
    int pCenterY = guiHeight / 2;

    // apply the vector to our center to get the coordinates in pixels
    px = pCenterX + vecX * guiWidth;
    py = pCenterY - vecY * guiHeight;

    // check if it's inside the window
    if (px >= 0 && px < guiWidth && py >= 0 && py < guiHeight) {
        return true;
    }
    return false;
}

bool XPlaneBaseWindow::isActive() {
    return (winHandle != nullptr);
}

void XPlaneBaseWindow::watchdog() {
    if (!winHandle) return;

    // XPLMGetWindowIsVisible() returns true even if the user closed the window.
    // So, if the window is supposed to be visible then check the watchdog counter.
    // It gets reset on every call to onDraw. If it reaches 10 then the window is not
    // being drawn. It might have been closed, or moved off-screen. Either way ...
    if (++noDrawWatchdog >= 10) {
        destroy();
        noDrawWatchdog = 0;
    }

}

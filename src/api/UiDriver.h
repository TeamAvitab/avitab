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

#include <string>
#include <cstdint>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>

namespace avitab {

struct WindowRect {
    bool valid = false;
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    bool poppedOut = false;
};

class Settings;
class AviTabCore;

// The UiDriverBase defines interfaces for the Avitab core to interact with the user
// using the mouse, scroll wheel, keyboard and a displayed image.
// Implementations are specialised for X-Plane, MSFS and a GLFW desktop application.

class UiDriverBase {
public:
    static const int INITIAL_WIDTH = 800;
    static const int INITIAL_HEIGHT = 480;

    UiDriverBase();
    virtual ~UiDriverBase();

public: // API must be implemented by derived class
    virtual void requestCloseWindow() = 0;

    virtual void blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t *data) = 0;

    virtual void readPointerState(int &x, int &y, bool &pressed) = 0;
    virtual int getWheelClicks() = 0;

    virtual void setBrightness(float b) = 0;
    virtual float getBrightness() = 0;

public: // API implemented by (this) base class
    void setWantKeyInput(bool wantKeys);
    uint32_t popKeyPress();

    using ResizeCallback = std::function<void(int, int)>;
    void setResizeCallback(ResizeCallback cb);

protected:
    uint32_t *data();
    int width();
    int height();
    void resize(int newWidth, int newHeight);

    bool wantsKeyInput();
    void pushKeyInput(uint32_t c);

private:
    ResizeCallback onResize;
    std::mutex keyMutex;
    bool enableKeyInput = false;
    std::queue<uint32_t> keyInput;
    std::atomic_int bufferWidth{ INITIAL_WIDTH }, bufferHeight{ INITIAL_HEIGHT };
    std::vector<uint32_t> buffer;
};

}

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

#include <GLFW/glfw3.h>
#include <vector>
#include <mutex>
#include <atomic>
#include "UiDriver.h"

class GlfwUiDriver: public avitab::UiDriverBase {
public:
    GlfwUiDriver();
    ~GlfwUiDriver();

    // The top-level state machine is start -> enable -> disable { -> enable -> disable } -> stop
    void start(std::shared_ptr<avitab::Settings> prefs, std::shared_ptr<avitab::AviTabCore> core);
    void enable();
    void disable();
    void stop();

    void setBrightness(float b) override;
    float getBrightness() override;

    bool handleEvents(bool &disableReq);
    uint32_t getLastDrawTime();

    void blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t *newData) override;
    void readPointerState(int &x, int &y, bool &pressed) override;
    int getWheelClicks() override;

    void requestCloseWindow() override;

protected:
    void createWindow(const std::string &title);

private:
    void createTexture();
    void render();

private:
    static constexpr const float ZOOM = 1.5f;

    std::mutex driverMutex;
    GLFWwindow *window {};
    GLuint textureId{};
    std::atomic<uint32_t> lastDrawTime {0};
    float brightness = 1;
    bool needsRedraw = false;
    bool disableRequested = false;

    std::atomic_int mouseX {0}, mouseY {0}, wheelDir {0};
    bool mousePressed {false};
};

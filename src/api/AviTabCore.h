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

#include <memory>

namespace avitab {

// Abstract interface to the AviTab core used by the simulation and windows drivers
// and the product wrappers.

class Settings;
class SimDriverBase;
class UiDriverBase;

class AviTabCore {
public:
    static std::shared_ptr<AviTabCore> CreateAviTabCore();

    virtual ~AviTabCore() = default;

    // APIs called by the product wrappers.
    // The top-level state machine is start -> enable -> disable { -> enable -> disable } -> stop
    // Start/stop are moore-or-less equivalent to construction/destruction
    // Enable/disable correspond to the X-Plane plugin events and can be iterated. Other products just call
    // enable once after start, and disable once before stop.
    virtual void start(std::shared_ptr<Settings> prefs, std::shared_ptr<SimDriverBase> sim, std::shared_ptr<UiDriverBase> ui) = 0;
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual void stop() = 0;

    // Called when a new plane has been loaded
    virtual void onPlaneLoad(bool hideHeaderApp) = 0;

    // APIs for injection of commands from the product, mainly from the X-Plane plugin
    virtual void commandShowApp(const std::string &appName) = 0;
    virtual void commandZoomIn() = 0;
    virtual void commandZoomOut() = 0;
    virtual void commandRecentre() = 0;
    virtual void commandPanLeft() = 0;
    virtual void commandPanRight() = 0;
    virtual void commandPanUp() = 0;
    virtual void commandPanDown() = 0;
    virtual void commandTabNext() = 0;
    virtual void commandTabPrevious() = 0;
    virtual void commandWheelUp(int x, int y) = 0;
    virtual void commandWheelDown(int x, int y) = 0;
};

} // namespace avitab

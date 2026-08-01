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

#include "ui/glfw/GlfwUiDriver.h"
#include <memory>
#include <map>
#include "ToolSimDriver.h"

class GlfwUiDriver;

class MockSimDriver : public ToolSimDriver
{
public:
    // Must be called from the simDriver thread - do not call from GUI thread!
    MockSimDriver();

    // avitab::SimDriverBase overrides

    std::shared_ptr<avitab::UiDriverBase> createUiDriver() override;

    void eventLoop();

    // Can be called from any thread
    std::string getMETARForAirport(const std::string &icao) override;
    int getWeatherAtLocation(const world::Location &loc, const float &altitude, std::string &weather) override;
    std::string getNearestAirportId() override;
    std::vector<float> getMagneticVariations(std::vector<world::Location> &locs) override;
    avitab::AircraftID getActiveAircraftCount() override;
    world::Position getAircraftPosition(avitab::AircraftID id) override;
    unsigned int getZuluTimeSeconds() override;
    unsigned int getLocalTimeSeconds() override;

    virtual ~MockSimDriver();

protected:
    std::shared_ptr<GlfwUiDriver> driver;

};

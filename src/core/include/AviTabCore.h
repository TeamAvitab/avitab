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

class Environment;
class GUIDriver;

class AviTabCore {
public:
    static std::unique_ptr<AviTabCore> CreateAviTabCore(std::shared_ptr<Environment> env, std::shared_ptr<GUIDriver> gui);

    virtual void startApp() = 0;
    virtual void stopApp() = 0;
    virtual void toggleTablet() = 0;
    virtual void onPlaneLoad() = 0;

    virtual ~AviTabCore() { }
};

} // namespace avitab

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
#include <memory>
#include <vector>
#include "core/world/LoadManager.h"
#include "env/xplane/libxdata/XWorld.h"
#include "env/xplane/libxdata/loaders/AirportLoader.h"

namespace xdata {

class XData : public world::LoadManager {
public:
    XData(const std::string &dataRootPath);
    virtual ~XData() = default;
    std::shared_ptr<world::World> getWorld() override;
    void discoverSceneries() override;
    void load() override;
    void reloadMetar() override;
private:
    std::string xplaneRoot;
    std::string navDataPath;
    std::shared_ptr<xdata::XWorld> xworld;
    std::vector<std::string> customSceneries;
    std::string userFixesFilename;

    std::string determineNavDataPath();

    void loadAirports();
    void loadFixes();
    void loadNavaids();
    void loadAirways();
    void loadProcedures();
    void loadMetar();
    void loadCustomScenery(const AirportLoader& loader);

};

} /* namespace xdata */

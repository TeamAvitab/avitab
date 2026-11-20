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
#include <filesystem>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "WorldGeometry.h"
#include "nav/parsers/xplane/NavaidParser.h"

namespace navdb {
class InMemoryNavDb;
class FPAPModel;
};

namespace xdata {

class XplaneNavDataLoader;

class NavaidLoader {
public:
    NavaidLoader(XplaneNavDataLoader* mgr);
    void load(const std::filesystem::path &file);
private:
    void onNavaidLoaded(const NavaidData &navaid);
    void onGLSLoaded(const NavaidData &navaid);
    void onThresholdLoaded(const NavaidData &navaid);
private:
    XplaneNavDataLoader * const loadMgr;
    std::shared_ptr<navdb::InMemoryNavDb> world;

    // FPAP (type 14) and LTP/FTP (type 16) of an approach share airport and procedure identifier
    using ApproachKey = std::pair<std::string, std::string>; // airport, procedure identifier
    std::map<ApproachKey, navdb::FPAPModel *> fpaps;           // owned by their fixes in the nav database
    struct Threshold {
        world::Location location;
        double glidepathAngle;
    };
    std::map<ApproachKey, Threshold> pendingThresholds; // LTP/FTP seen before its FPAP
    // GLS ground stations (type 15), turned into approaches once all FPAPs are known
    std::vector<NavaidData> pendingGLSs;
    void addPendingGLSs();
    int skippedFPAPs = 0;
    int loadedFPAPs = 0;
    int loadedGLSs = 0;

};

} /* namespace xdata */

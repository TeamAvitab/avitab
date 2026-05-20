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
#include "UserFixLoader.h"
#include "nav/models/UserFix.h"
#include "nav/parsers/UserFixParser.h"
#include "WorldGeometry.h"
#include "Logger.h"
#include <cassert>

namespace navdb {

std::vector<std::shared_ptr<UserFix>> UserFixLoader::load(const std::filesystem::path& file) {
    UserFixParser parser(file);
    parser.setAcceptor([this] (const UserFixData &data) {
        try {
            onUserFixLoaded(data);
        } catch (const std::exception &e) {
            logger::warn("Can't parse userfix %s: %s", data.ident.c_str(), e.what());
        }
    });
    parser.loadUserFixes();
    return fixes;
}

void UserFixLoader::onUserFixLoaded(const UserFixData& userfixdata) {
    // User fixes are unique, so create new fix each time
    // No region in LNM/PlanG csv format, so just use dummy one
    auto loc = world::Location::fromGCS(userfixdata.latitude, userfixdata.longitude);
    assert(userfixdata.ident.length());
    auto fix = std::make_shared<UserFixModel>(userfixdata.type, userfixdata.name, userfixdata.ident, loc);
    fixes.push_back(fix);
}

} /* namespace navdb */

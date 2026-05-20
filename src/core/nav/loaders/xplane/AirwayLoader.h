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

namespace navdb {
class InMemoryNavDb;
};

namespace xdata {

class XplaneNavDataLoader;

class AirwayData;

class AirwayLoader {
public:
    AirwayLoader(XplaneNavDataLoader * mgr);
    void load(const std::filesystem::path &file);
private:
    void onAirwayLoaded(const AirwayData &airway);
private:
    XplaneNavDataLoader * const loadMgr;
    std::shared_ptr<navdb::InMemoryNavDb> world;
};

} /* namespace xdata */

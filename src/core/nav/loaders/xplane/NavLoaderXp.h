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

#include <filesystem>
#include <string>
#include <memory>
#include <vector>

namespace navdb {
class NavDatabase;
class InMemoryNavDb;
class NavDbManager;
}

namespace xdata {

class AirportLoader;

class XplaneNavDataLoader {
public:
    XplaneNavDataLoader(const navdb::NavDbManager &parent, const std::filesystem::path & xpInstallRoot);
    virtual ~XplaneNavDataLoader() = default;
    void discoverSceneries();
    void load();
    bool shouldCancelLoading();
    std::shared_ptr<navdb::InMemoryNavDb> getDatabase() { return xpnavdb; }

private:
    void loadAirports();
    void loadFixes();
    void loadNavaids();
    void loadAirways();
    void loadProcedures();
    void loadCustomScenery();

private:
    const navdb::NavDbManager &owner;
    const std::filesystem::path xpInstallRoot;
    std::filesystem::path navDataPath;
    unsigned xpVersion;
    std::shared_ptr<navdb::InMemoryNavDb> xpnavdb;
    std::vector<std::filesystem::path> customSceneries;
};

} /* namespace xdata */

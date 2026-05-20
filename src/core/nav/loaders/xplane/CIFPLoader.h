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
#include "Navigation.h"
#include "nav/parsers/xplane/CIFPParser.h"

namespace navdb {
class InMemoryNavDb;
class AirportModel;
class RunwayModel;
class ApproachModel;
class ProcedureOptions;
};

namespace xdata {

class XplaneNavDataLoader;

class CIFPLoader {
public:
    CIFPLoader(XplaneNavDataLoader * mgr);
    void load(std::shared_ptr<navdb::AirportModel> airport, const std::filesystem::path &file);
private:
    void onProcedureLoaded(std::shared_ptr<navdb::AirportModel> airport, const CIFPData &procedure);

    void loadRunway(std::shared_ptr<navdb::AirportModel> airport, const CIFPData &procedure);
    void loadSID(std::shared_ptr<navdb::AirportModel> airport, const CIFPData &procedure);
    void loadSTAR(std::shared_ptr<navdb::AirportModel> airport, const CIFPData &procedure);
    void loadApproach(std::shared_ptr<navdb::AirportModel> airport, const CIFPData &procedure);

    navdb::NodeList convertFixes(std::shared_ptr<navdb::AirportModel> airport, const std::vector<CIFPData::FixInRegion> &fixes) const;
    void loadRunwayTransition(const CIFPData& procedure, navdb::ProcedureOptions &trns, const std::shared_ptr<navdb::AirportModel>& airport);
    void loadCommonRoutes(const CIFPData& procedure, navdb::ProcedureOptions &trns, const std::shared_ptr<navdb::AirportModel>& airport);
    void loadEnroute(const CIFPData& procedure, navdb::ProcedureOptions &trns, const std::shared_ptr<navdb::AirportModel>& airport);
    void loadApproachTransitions(const CIFPData& procedure, navdb::ApproachModel &appr, const std::shared_ptr<navdb::AirportModel>& airport);
    void loadApproaches(const CIFPData& procedure, navdb::ApproachModel &appr, const std::shared_ptr<navdb::AirportModel>& airport);

    void forEveryMatchingRunway(const std::string &rwSpec, const std::shared_ptr<navdb::AirportModel> apt, std::function<void (std::shared_ptr<navdb::RunwayModel>)> f);
private:
    XplaneNavDataLoader * const loadMgr;
    std::shared_ptr<navdb::InMemoryNavDb> world;
};

} /* namespace xdata */

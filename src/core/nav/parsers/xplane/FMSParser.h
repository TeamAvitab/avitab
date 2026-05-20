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
#include <functional>
#include "../BaseParser.h"

namespace xdata {

struct FlightPlanNodeData {
    enum class Type {
        ERR = 0,
        AIRPORT,
        NDB,
        VOR,
        FIX,
        UNNAMED,
        CYCLE,
        ADEP,
        DEP,
        DEPRWY,
        SID,
        SIDTRANS,
        ADES,
        DESRWY,
        STAR,
        STARTRANS,
        APP,
        APPTRANS
    };

    int lineNum;
    std::string line;

    Type type;
    std::string id;
    std::string via;
    double alt = std::numeric_limits<double>::quiet_NaN();
    double lat = std::numeric_limits<double>::quiet_NaN();
    double lon = std::numeric_limits<double>::quiet_NaN();
};

class FMSParser {
public:
    using Acceptor = std::function<void(const FlightPlanNodeData &)>;

    FMSParser(const std::filesystem::path &fmsFilename);
    void setAcceptor(Acceptor a);
    std::string getHeader() const;
    void loadFMS();
private:
    Acceptor acceptor;
    std::string header;
    navdb::BaseParser parser;
    int lineNum;
    bool parsingEnRouteBlock;

    void parseLine();
    void parseEnRouteBlock();
    void parseIntroBlocks();
    FlightPlanNodeData::Type parseWaypointType(int num);
};

} /* namespace xdata */

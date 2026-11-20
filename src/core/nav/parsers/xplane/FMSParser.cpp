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

#include <cmath>
#include <iostream>
#include <map>
#include "FMSParser.h"
#include "Logger.h"
#include "platform/cstrtod.h"

namespace xdata {

FMSParser::FMSParser(const std::filesystem::path& fmsFilename):
    parser(fmsFilename)
{
    parsingEnRouteBlock = false;
    header = parser.parseHeader();
    lineNum = 3;
}

std::string FMSParser::getHeader() const {
    return header;
}

void FMSParser::setAcceptor(Acceptor a) {
    acceptor = a;
}

void FMSParser::loadFMS() {
    using namespace std::placeholders;
    if (parser.getVersion() == 3) {
        parser.consumeLine();
        parser.consumeLine();
        lineNum += 2;
        parsingEnRouteBlock = true;
    }
    parser.eachLine(std::bind(&FMSParser::parseLine, this));
}

void FMSParser::parseLine() {
    if (parsingEnRouteBlock) {
        parseEnRouteBlock();
    } else {
        parseIntroBlocks();
    }
    lineNum++;
}

void FMSParser::parseEnRouteBlock() {
    FlightPlanNodeData node {};
    node.type = parseWaypointType(parser.parseInt());
    if (node.type == FlightPlanNodeData::Type::ERR) {
        return;
    }
    node.id = parser.parseWord();
    if (parser.getVersion() > 3) {
        node.via = parser.parseWord();
    }
    node.alt = parser.parseDouble();
    node.lat = parser.parseDouble();
    node.lon = parser.parseDouble();

    acceptor(node);
}

void FMSParser::parseIntroBlocks() {
    static const std::map<std::string, FlightPlanNodeData::Type> introTypes = {
        {"CYCLE", FlightPlanNodeData::Type::CYCLE},
        {"ADEP", FlightPlanNodeData::Type::ADEP},
        {"DEP", FlightPlanNodeData::Type::ADEP},
        {"DEPRWY", FlightPlanNodeData::Type::DEPRWY},
        {"SID", FlightPlanNodeData::Type::SID},
        {"SIDTRANS", FlightPlanNodeData::Type::SIDTRANS},
        {"APP", FlightPlanNodeData::Type::APP},
        {"APPTRANS", FlightPlanNodeData::Type::APPTRANS},
        {"ADES", FlightPlanNodeData::Type::ADES},
        {"DES", FlightPlanNodeData::Type::ADES},
        {"DESRWY", FlightPlanNodeData::Type::DESRWY},
        {"STAR", FlightPlanNodeData::Type::STAR},
        {"STARTRANS", FlightPlanNodeData::Type::STARTRANS}
    };

    FlightPlanNodeData node {};
    std::string prefix = parser.parseWord();
    if (prefix == "NUMENR") {
        parsingEnRouteBlock = true;
    } else {
        const auto type = introTypes.find(prefix);
        if (type == introTypes.end()) {
            logger::warn("Unknown FMS entry type %s @ line %d", prefix.c_str(), lineNum);
        } else {
            node.type = type->second;
            node.id = parser.parseWord();
            acceptor(node);
        }
    }
}

FlightPlanNodeData::Type FMSParser::parseWaypointType(int num) {
    switch (num) {
    case 1:   return FlightPlanNodeData::Type::AIRPORT;
    case 2:   return FlightPlanNodeData::Type::NDB;
    case 3:   return FlightPlanNodeData::Type::VOR;
    case 4:   return FlightPlanNodeData::Type::FIX; // Version 3 FMS
    case 11:  return FlightPlanNodeData::Type::FIX;
    case 28:  return FlightPlanNodeData::Type::UNNAMED;
    default:
        logger::warn("Unknown flight plan node type: %d @ line %d", num, lineNum);
        return FlightPlanNodeData::Type::ERR;
    }
}

} /* namespace xdata */

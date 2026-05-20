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

#include "SceneryIndexParser.h"

namespace xdata {

SceneryIndexParser::SceneryIndexParser(const std::filesystem::path &file):
    parser(file)
{
}

void SceneryIndexParser::setAcceptor(Acceptor a) {
    acceptor = a;
}

void SceneryIndexParser::loadCustomScenery() {
    using namespace std::placeholders;
    parser.eachLine(std::bind(&SceneryIndexParser::parseLine, this));
}

void SceneryIndexParser::parseLine() {
    auto firstWord = parser.parseWord();

    if (firstWord.empty() || firstWord != "SCENERY_PACK") {
        return;
    }

    parser.skipWhiteSpace();

    curData = parser.restOfLine();
    acceptor(curData);
    curData = {};
}

} /* namespace xdata */

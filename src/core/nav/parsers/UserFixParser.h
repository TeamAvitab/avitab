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
#include "BaseParser.h"
#include "Navigation.h"

namespace navdb {


struct UserFixData {
    UserFixType type = UserFixType::NONE;
    std::string name;
    std::string ident;
    double latitude = std::numeric_limits<double>::quiet_NaN();
    double longitude = std::numeric_limits<double>::quiet_NaN();
};

class UserFixParser {
public:
    using Acceptor = std::function<void(const UserFixData &)>;

    UserFixParser(const std::filesystem::path &file);
    void setAcceptor(Acceptor a);
    std::string getHeader() const;
    void loadUserFixes();
private:
    Acceptor acceptor;
    std::string header;
    BaseParser parser;
    int lineNum;

    void parseLine();
    UserFixType parseType(std::string& typeString);
};

} /* namespace navdb */

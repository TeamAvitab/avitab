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

namespace navdb {

struct UserFixData;

// The user fix loader is slightly different from the other navigation
// data loaders because the user fixes can be updated at any time, so they
// are, in essence, independent to whichever NAV database is in use.

class UserFixLoader
{
public:
    UserFixLoader() = default;
    std::vector<std::shared_ptr<UserFix>> load(const std::filesystem::path &file);

private:
    void onUserFixLoaded(const UserFixData &navaid);

private:
    std::vector<std::shared_ptr<UserFix>> fixes;
};

} /* namespace navdb */

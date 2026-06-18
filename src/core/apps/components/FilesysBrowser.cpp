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
#include "FilesysBrowser.h"
#include "Logger.h"

namespace avitab {

FilesystemBrowser::FilesystemBrowser(const std::filesystem::path & s) :
    start(s)
{
    if (!std::filesystem::exists(start)) {
        logger::error("File browser started in non-existent directory %s", start.u8string().c_str());
    }
    goTo(start);
}

void FilesystemBrowser::goUp() {
    try {
        auto up = std::filesystem::canonical(cwd.parent_path());
        if (std::filesystem::exists(up)) {
            cwd = up;
        }
    } catch (...) {
        // if going up fails, go back to the start
        cwd = start;
    }
}

void FilesystemBrowser::goTo(const std::filesystem::path &dir) {
    try {
        auto nd = dir.has_root_path() ? dir : cwd / dir;
        if (std::filesystem::exists(nd)) {
            cwd = std::filesystem::canonical(nd);
        }
    } catch (...) {
        // leave current directory unchanged
    }
}

void FilesystemBrowser::setFilter(const std::string &regex) {
    filterRegex = std::regex(regex, std::regex_constants::ECMAScript | std::regex_constants::icase);
}

std::filesystem::path FilesystemBrowser::path() {
    return cwd;
}

std::string FilesystemBrowser::rtrimmed(const size_t max) {
    auto s = platform::pathToDisplayString(cwd);
    if (s.size() <= max) {
        return s;
    }
    return std::string("...") + s.substr(s.size() - (max - 3));
}

std::vector<platform::DirEntry> FilesystemBrowser::entries(bool applyFilter, bool sort) {
    items.clear();
    try {
        items = platform::readDirectory(cwd);
        if (applyFilter) {
            filterEntries();
        }
        if (sort) {
            sortEntries();
        }
    } catch (const std::exception &e) {
        logger::error("Couldn't read directory %s: %s", cwd.c_str(), e.what());
    }

    return items;
}

void FilesystemBrowser::filterEntries() {
    auto iter = std::remove_if(std::begin(items), std::end(items), [this] (const auto &a) -> bool {
        if (a.isDirectory) {
            return false;
        }
        return !std::regex_search(a.utf8Name.u8string(), filterRegex);
    });
    items.erase(iter, std::end(items));
}

void FilesystemBrowser::sortEntries() {
    auto comparator = [] (const platform::DirEntry &a, const platform::DirEntry &b) -> bool {
        if (a.isDirectory && !b.isDirectory) {
            return true;
        }

        if (!a.isDirectory && b.isDirectory) {
            return false;
        }

        return a.utf8Name < b.utf8Name;
    };

    std::sort(begin(items), end(items), comparator);
}

}

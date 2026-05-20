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
#include "ToolEnvironment.h"
#include "platform/Platform.h"

ToolEnvironment::ToolEnvironment() : Environment() {
    xplaneRootPath = findXPlaneInstallationPath();
    ourPath = platform::getExectuablePath();
}

void ToolEnvironment::createMenu(const std::string& name) {
}

void ToolEnvironment::addMenuEntry(const std::string& label, MenuCallback cb) {
}

void ToolEnvironment::destroyMenu() {
}

void ToolEnvironment::createCommand(const std::string& name, const std::string& desc, CommandCallback cb) {
}

void ToolEnvironment::destroyCommands() {
}

std::filesystem::path ToolEnvironment::getAirplanePath() {
    return ourPath;
}

std::filesystem::path ToolEnvironment::getProgramPath() {
    return ourPath;
}

std::filesystem::path ToolEnvironment::getDataRootPath() {
    return ourPath;
}

std::filesystem::path ToolEnvironment::getSettingsDir() {
    return ourPath;
}

std::filesystem::path ToolEnvironment::getFlightPlansPath() {
    return ourPath;
}

std::filesystem::path ToolEnvironment::getFontDirectory() {
    return ourPath;
}

std::filesystem::path ToolEnvironment::getXpNavDataRootPath() {
    return xplaneRootPath;
}

std::filesystem::path ToolEnvironment::getMsfsNavDataRootPath() {
    return "";
}

std::filesystem::path ToolEnvironment::findXPlaneInstallationPath()
{
    // When running in the tool environment we can't ask X-Plane where to find
    // the NAV data. The installer creates a text file, so we look for this and
    // then find the most recent installation.

    std::filesystem::path installFilePath;

    switch (platform::getPlatform()) {
    case platform::Platform::WINDOWS:
        installFilePath = std::filesystem::u8path(getenv("LOCALAPPDATA"));
        break;
    case platform::Platform::MAC:
        installFilePath = std::filesystem::u8path(getenv("HOME")) / "Library" / "Preferences";
        break;
    case platform::Platform::LINUX:
        installFilePath = std::filesystem::u8path(getenv("HOME")) / ".x-plane";
        break;
    }

    std::list<std::filesystem::path> candidates;
    candidates.push_back(""); // fallback if no other entries are found

    // we'll build the list in reverse priority order
    std::ifstream xp11if(installFilePath / "x-plane_install_11.txt");
    while (xp11if.good()) {
        std::string installDir;
        std::getline(xp11if, installDir);
        auto xp11p = std::filesystem::u8path(installDir);
        if (std::filesystem::exists(xp11p/"Resources"/"default scenery"/"default apt dat"/"Earth nav data"/"apt.dat")) {
            candidates.push_front(xp11p);
        }
    }

    std::ifstream xp12if(installFilePath / "x-plane_install_12.txt");
    while (xp12if.good()) {
        std::string installDir;
        std::getline(xp12if, installDir);
        auto xp12p = std::filesystem::u8path(installDir);
        if (std::filesystem::exists(xp12p/"Global Scenery"/"Global Airports"/"Earth nav data"/"apt.dat")) {
            candidates.push_front(xp12p);
        }
    }

    // use the most recent one we found, or the fallback
    return candidates.front();
}

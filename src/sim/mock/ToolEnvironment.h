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

#include "Environment.h"

/**
 * This class implements methods common to desktop/tool variants of Avitab,
 * currently the XPlane stand-alone application (used for testing) and the MS
 * Flight Simulator in-game panel server for Avitab.
 */
class ToolEnvironment : public avitab::Environment {
public:
    ToolEnvironment();

    void createMenu(const std::string &name) override;
    void addMenuEntry(const std::string &label, std::function<void()> cb) override;
    void destroyMenu() override;
    void createCommand(const std::string &name, const std::string &desc, CommandCallback cb) override;
    void destroyCommands() override;
    void enableAndPowerPanel() override { }

    std::filesystem::path getProgramPath() override;
    std::filesystem::path getDataRootPath() override;
    std::filesystem::path getSettingsDir() override;
    std::filesystem::path getFlightPlansPath() override;
    std::filesystem::path getFontDirectory() override;
    std::filesystem::path getAirplanePath() override;
    std::filesystem::path getXpNavDataRootPath() override;
    std::filesystem::path getMsfsNavDataRootPath() override;


    void onAircraftReload() override { }

    void setIsInMenu(bool menu) override { }
    void updateMapExports(float lat, float lon, int zoom, float vrange) override { }

protected:
    std::filesystem::path xplaneRootPath;
    std::filesystem::path ourPath;

private:
    std::filesystem::path findXPlaneInstallationPath();

};

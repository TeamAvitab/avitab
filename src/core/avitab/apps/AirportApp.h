/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2025 Folke Will
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
#include <vector>
#include "App.h"
#include "core/gui_toolkit/widgets/TextArea.h"
#include "core/gui_toolkit/widgets/Keyboard.h"
#include "core/gui_toolkit/widgets/TabGroup.h"
#include "core/gui_toolkit/widgets/Label.h"
#include "core/gui_toolkit/widgets/Page.h"
#include "core/gui_toolkit/widgets/Button.h"
#include "core/gui_toolkit/widgets/Checkbox.h"
#include "core/gui_toolkit/widgets/Container.h"
#include "core/gui_toolkit/widgets/PixMap.h"
#include "core/gui_toolkit/widgets/DropDownList.h"
#include "core/gui_toolkit/widgets/List.h"
#include "core/gui_toolkit/widgets/Window.h"
#include "core/gui_toolkit/widgets/MessageBox.h"
#include "core/gui_toolkit/Timer.h"
#include "core/libimg/stitcher/TileSource.h"
#include "core/maps/OverlayedMap.h"
#include "core/libimg/stitcher/Stitcher.h"

namespace avitab {

class AirportApp: public App {
public:
    AirportApp(FuncsPtr appFuncs);
    void onMouseWheel(int dir, int x, int y) override;
    void changeChartTab(bool next) override;
private:
    struct TabPage {
        std::shared_ptr<Page> page;
        std::shared_ptr<Window> window;
        std::shared_ptr<Label> label;
        std::shared_ptr<Button> trackButton, nightModeButton;
        std::shared_ptr<world::Airport> airport;

        apis::ChartCategory requestedList = apis::ChartCategory::ROOT;
        apis::ChartService::ChartList charts;
        std::shared_ptr<List> chartSelect;

        std::shared_ptr<apis::Chart> chart;
        std::shared_ptr<img::TileSource> mapSource;
        std::shared_ptr<img::Image> mapImage;
        std::shared_ptr<img::Stitcher> mapStitcher;
        std::shared_ptr<maps::OverlayedMap> map;
        std::shared_ptr<PixMap> pixMap;
        std::shared_ptr<maps::OverlayConfig> overlays;

        int panPosX = 0, panPosY = 0;
        bool trackPlane = true;
    };
    std::vector<TabPage> pages;
    bool nightMode = false;

    Timer updateTimer;
    std::shared_ptr<Page> searchPage;
    std::shared_ptr<Window> searchWindow;
    std::shared_ptr<Container> prefContainer;
    std::shared_ptr<Label> searchLabel, sortLabel;
    std::shared_ptr<Checkbox> sortCheckbox, sortAscCheckbox;
    std::shared_ptr<TabGroup> tabs;
    std::shared_ptr<TextArea> searchField;
    std::shared_ptr<DropDownList> resultList;
    std::shared_ptr<Button> nextButton;
    std::shared_ptr<Keyboard> keys;
    std::shared_ptr<Button> nearestButton;
    std::shared_ptr<avitab::AirportConfig> airportConfig;

    void removeTab(std::shared_ptr<Page> page);
    void resetLayout();
    void onSearchEntered(const std::string &code);
    void onAirportSelected(std::shared_ptr<world::Airport> airport);
    void clearSearch();
    void sortSearchResults(std::vector<std::shared_ptr<world::Airport>> &airports);
    void fillPage(std::shared_ptr<Page> page, std::shared_ptr<world::Airport> airport);

    std::string getDisplayID(std::shared_ptr<world::Airport> airport);
    std::tuple<double, double> getNavData(std::shared_ptr<world::Airport> airport);
    std::string toAptHeader(std::shared_ptr<world::Airport> airport);
    std::string toATCInfo(std::shared_ptr<world::Airport> airport);
    std::string toATCString(const std::string &name, std::shared_ptr<world::Airport> airport, world::Airport::ATCFrequency type);
    std::string toRunwayInfo(std::shared_ptr<world::Airport> airport);
    std::string toHeliportInfo(std::shared_ptr<world::Airport> airport);
    std::string toWeatherInfo(std::shared_ptr<world::Airport> airport);

    TabPage &findPage(std::shared_ptr<Page> page);

    void toggleCharts(std::shared_ptr<Page> page, std::shared_ptr<world::Airport> airport);
    void fillChartsPage(std::shared_ptr<Page> page, std::shared_ptr<world::Airport> airport);
    void onChartsLoaded(std::shared_ptr<Page> page, const apis::ChartService::ChartList &charts);
    void onChartLoaded(std::shared_ptr<Page> page);
    void createSettingsContainer();
    void toggleSettings();
    void onMapPan(std::shared_ptr<Page> page, int x, int y, bool start, bool end);
    void redrawPage(std::shared_ptr<Page> page);
    bool onTimer();

    size_t countCharts(const apis::ChartService::ChartList &list, apis::ChartCategory category);
};

} /* namespace avitab */

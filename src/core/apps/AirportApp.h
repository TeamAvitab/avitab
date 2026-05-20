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
#include <vector>
#include "App.h"
#include "gui/widgets/TextArea.h"
#include "gui/widgets/Keyboard.h"
#include "gui/widgets/TabGroup.h"
#include "gui/widgets/Label.h"
#include "gui/widgets/Page.h"
#include "gui/widgets/Button.h"
#include "gui/widgets/Checkbox.h"
#include "gui/widgets/Container.h"
#include "gui/widgets/PixMap.h"
#include "gui/widgets/DropDownList.h"
#include "gui/widgets/List.h"
#include "gui/widgets/Window.h"
#include "gui/widgets/MessageBox.h"
#include "gui/Timer.h"
#include "image/stitcher/TileSource.h"
#include "maps/OverlayedMap.h"
#include "image/stitcher/Stitcher.h"

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
        std::shared_ptr<navdb::Airport> airport;

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
    void onAirportSelected(std::shared_ptr<navdb::Airport> airport);
    void clearSearch();
    void sortSearchResults(std::vector<std::shared_ptr<navdb::Airport>> &airports);
    void fillPage(std::shared_ptr<Page> page, std::shared_ptr<navdb::Airport> airport);

    std::string getDisplayID(std::shared_ptr<navdb::Airport> airport);
    std::tuple<double, double> getNavData(std::shared_ptr<navdb::Airport> airport);
    std::string toAptHeader(std::shared_ptr<navdb::Airport> airport);
    std::string toATCInfo(std::shared_ptr<navdb::Airport> airport);
    std::string toATCString(const std::string &name, std::shared_ptr<navdb::Airport> airport, navdb::ATCserviceType type);
    std::string toRunwayInfo(std::shared_ptr<navdb::Airport> airport);
    std::string toHeliportInfo(std::shared_ptr<navdb::Airport> airport);
    std::string toWeatherInfo(std::shared_ptr<navdb::Airport> airport);

    TabPage &findPage(std::shared_ptr<Page> page);

    void toggleCharts(std::shared_ptr<Page> page, std::shared_ptr<navdb::Airport> airport);
    void fillChartsPage(std::shared_ptr<Page> page, std::shared_ptr<navdb::Airport> airport);
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

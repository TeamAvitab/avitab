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

// REFACTOR - class AviTab is a muddle of core, simDriver and uiDriver.
// It needs reworking.

#include <climits>
#include <future>
#include <filesystem>
#include <memory>
#include "AviTabCore.h"
#include "apps/AppFunctions.h"
#include "UiDriver.h"
#include "SimDriver.h"
#include "platform/Platform.h"
#include "Logger.h"
#include "JsonConfig.h"
#include "gui/LVGLToolkit.h"
#include "image/TTFStamper.h"
#include "apps/HeaderApp.h"
#include "apps/AppLauncher.h"
#include "AviTabBuildSettings.h"
#include "nav/NavDbManager.h"
#include "nav/routing/RouteFinder.h"


static const char *defaultMapConfigJson();

namespace avitab {

class AviTab : public AviTabCore, public AppFunctions {
public:
    AviTab(std::shared_ptr<SimDriverBase> sim, std::shared_ptr<UiDriverBase> gui);
    void startApp() override;
    void toggleTablet() override;
    void resetWindowPosition();
    void zoomIn();
    void zoomOut();
    void recentre();
    void panLeft();
    void panRight();
    void panUp();
    void panDown();
    void stopApp() override;
    void onPlaneLoad() override;

    // App API
    void setBrightness(float brightness) override;
    float getBrightness() override;
    void executeLater(std::function<void()> func) override;
    std::filesystem::path getAvitabInstallDir() override;
    std::filesystem::path getAvitabDataDir() override;
    std::filesystem::path getAirplanePath() override;
    std::filesystem::path getFlightPlansPath() override;
    std::shared_ptr<Container> createGUIContainer() override;
    void showGUIContainer(std::shared_ptr<Container> container) override;
    void onHomeButton() override;
    std::shared_ptr<navdb::NavDatabase> getNavDatabase() override;
    using MagVarMap = std::map<std::pair<double, double>, double>;
    std::vector<float> getMagneticVariations(std::vector<world::Location> &locs) override;
    std::string getMETARForAirport(const std::string &icao) override;
    int getWeatherAtLocation(const world::Location &loc, const float &altitude, std::string &weather) override;
    std::string getNearestAirportId() override;
    void loadUserFixes(const std::filesystem::path &filename) override;
    void close() override;
    void setIsInMenu(bool inMenu) override;
    std::shared_ptr<apis::ChartService> getChartService() override;
    AircraftID getActiveAircraftCount() override;
    world::Position getAircraftPosition(AircraftID id) override;
    unsigned int getFramesPerSecond() override;
    std::shared_ptr<Settings> getSettings() override;
    std::shared_ptr<navdb::Route> getRoute() override;
    void setRoute(std::shared_ptr<navdb::Route> route) override;
    void updateMapExports(float lat, float lon, int zoom, float vrange) override;
    unsigned int getZuluTimeSeconds() override;
    unsigned int getLocalTimeSeconds() override;

    ~AviTab();

private:
    bool hideHeader = false;
    std::shared_ptr<SimDriverBase> simDriver;
    std::shared_ptr<UiDriverBase> uiDriver;
    std::shared_ptr<LVGLToolkit> guiLib;

    std::unique_ptr<navdb::NavDbManager> navManager;

    std::shared_ptr<Label> loadLabel;

    std::shared_ptr<Container> headContainer;
    std::shared_ptr<Container> centerContainer;

    std::shared_ptr<App> headerApp;
    std::shared_ptr<AppLauncher> appLauncher;
    std::shared_ptr<navdb::Route> activeRoute;

    std::shared_ptr<apis::ChartService> chartService;
    bool resetWindowRect = false;

    void finishInstall();

    void createPanel();
    void createLayout();
    void showAppLauncher();
    void showApp(AppId id);
    void cleanupLayout();

    void onScreenResize();
    void handleClickCommand(bool down, bool drag);
    void handleWheelUpCommand();
    void handleWheelDownCommand();
    void changeChartTab(bool next);
};

// Factory
std::unique_ptr<AviTabCore> AviTabCore::CreateAviTabCore(std::shared_ptr<SimDriverBase> sim, std::shared_ptr<UiDriverBase> gui) {
    return std::make_unique<AviTab>(sim, gui);
}

AviTab::AviTab(std::shared_ptr<SimDriverBase> e, std::shared_ptr<UiDriverBase> gd):
    simDriver(e),
    uiDriver(gd)
{
    // runs in simDriver thread, called by XPluginEnable
    // NOTE order here is important. NAV db must be created before GUI is started.
    navManager = std::make_unique<navdb::NavDbManager>(simDriver->getXpNavDataRootPath(), simDriver->getMsfsNavDataRootPath());
    guiLib = std::make_shared<LVGLToolkit>(uiDriver);
    img::TTFStamper::setFontDirectory(simDriver->getFontDirectory());
    std::vector<std::string> remote_georefs_urls = simDriver->getSettings()->getGeneralSetting<std::vector<std::string>>("remote_georefs_urls");
    chartService = std::make_shared<apis::ChartService>(simDriver->getDataRootPath(), remote_georefs_urls);
    simDriver->resumeSimDriverJobs();
}

void AviTab::startApp() {
    // runs in simDriver thread, called by XPluginEnable
    logger::verbose("Starting AviTab %s", AVITAB_VERSION_STR);

    finishInstall();

    simDriver->createMenu("AviTab");
    simDriver->createCommand("AviTab/toggle_tablet", "Toggle Tablet", [this] (CommandState s) { if (s == CommandState::START) toggleTablet(); });
    simDriver->createCommand("AviTab/zoom_in", "Zoom In", [this] (CommandState s) { if (s == CommandState::START) zoomIn(); });
    simDriver->createCommand("AviTab/zoom_out", "Zoom Out", [this] (CommandState s) { if (s == CommandState::START) zoomOut(); });
    simDriver->createCommand("AviTab/recentre", "Recentre", [this] (CommandState s) { if (s == CommandState::START) recentre(); });
    simDriver->createCommand("AviTab/pan_left", "Pan left", [this] (CommandState s) { if (s == CommandState::START) panLeft(); });
    simDriver->createCommand("AviTab/pan_right", "Pan right", [this] (CommandState s) { if (s == CommandState::START) panRight(); });
    simDriver->createCommand("AviTab/pan_up", "Pan up", [this] (CommandState s) { if (s == CommandState::START) panUp(); });
    simDriver->createCommand("AviTab/pan_down", "Pan down", [this] (CommandState s) { if (s == CommandState::START) panDown(); });
    simDriver->createCommand("AviTab/Home", "Home Button",[this] (CommandState s) { if (s == CommandState::START) onHomeButton(); });

    // App commands
    simDriver->createCommand("AviTab/app_charts", "Charts App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::CHARTS); });
    simDriver->createCommand("AviTab/app_airports", "Airports App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::AIRPORTS); });
    simDriver->createCommand("AviTab/app_routes", "Routes App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::ROUTES); });
    simDriver->createCommand("AviTab/app_maps", "Maps App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::MAPS); });
    simDriver->createCommand("AviTab/app_plane_manual", "Plane Manual App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::PLANE_MANUAL); });
    simDriver->createCommand("AviTab/app_notes", "Notes App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::NOTES); });
    simDriver->createCommand("AviTab/app_navigraph", "Navigraph App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::NAVIGRAPH); });
    simDriver->createCommand("AviTab/app_about", "About App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::ABOUT); });
    simDriver->createCommand("AviTab/chart_tab_next", "Chart tab next", [this] (CommandState s) { if (s == CommandState::START) changeChartTab(true); });
    simDriver->createCommand("AviTab/chart_tab_prev", "Chart tab previous", [this] (CommandState s) { if (s == CommandState::START) changeChartTab(false); });

    // Direct control from panel integrations
    simDriver->createCommand("AviTab/click_left", "Left click", [this] (CommandState s) { handleClickCommand(s == CommandState::START, s == CommandState::CONTINUE); });
    simDriver->createCommand("AviTab/wheel_up", "Wheel up", [this] (CommandState s) { if (s == CommandState::START) handleWheelUpCommand(); });
    simDriver->createCommand("AviTab/wheel_down", "Wheel down", [this] (CommandState s) { if (s == CommandState::START) handleWheelDownCommand(); });

    // X-Plane menu items
    simDriver->addMenuEntry("Toggle Tablet", [this] { toggleTablet(); });
    simDriver->addMenuEntry("Reset Position", [this] { resetWindowPosition(); });

    guiLib->setMouseWheelCallback([this] (int dir, int x, int y) {
        if (appLauncher) {
            appLauncher->onMouseWheel(dir, x, y);
        }
    });
    createPanel();
    guiLib->executeLater(std::bind(&AviTab::createLayout, this));

    std::string userfixes_file = simDriver->getSettings()->getGeneralSetting<std::string>("userfixes_file");
    navManager->loadUserFixes(userfixes_file);

}

void AviTab::toggleTablet() {
    // runs in simDriver thread, called by menu or command
    try {
        if (!guiLib->hasNativeWindow()) {
            logger::info("Showing tablet");
            // It's possible that the user closed the window with the close button.
            // Since we don't get any callback for this, it's possible that we didn't store the last window coordinates yet.
            // For that reason, the last known position is tried first.
            auto rect = guiLib->getNativeWindowRect();
            if (rect.valid && !resetWindowRect) {
                simDriver->getSettings()->saveWindowRect(rect);
            } else {
                rect = simDriver->getSettings()->getWindowRect();
            }
            guiLib->createNativeWindow(std::string("Aviator's Tablet  ") + AVITAB_VERSION_STR, rect);
        } else {
            close();
        }
    } catch (const std::exception &e) {
        logger::error("Exception in onShowTablet: %s", e.what());
    }
}

void AviTab::resetWindowPosition() {
    // runs in simDriver thread
    simDriver->getSettings()->saveWindowRect({});
    if (guiLib->hasNativeWindow()) {
        guiLib->pauseNativeWindow();
    }
    resetWindowRect = true;
    toggleTablet();
    resetWindowRect = false;
}

void AviTab::onPlaneLoad() {
    // runs in simDriver thread
    // close on plane reload to reset the VR window position
    close();
    simDriver->onAircraftReload();
    createPanel();

    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->onPlaneLoad();
        }
        auto screen = guiLib->screen();
        if (hideHeader) {
            headerApp.reset();
            headContainer.reset();
            if (centerContainer) {
                centerContainer->setPosition(0, 0);
                centerContainer->setDimensions(screen->getWidth(), screen->getHeight());
            }
        } else {
            if (!headerApp) {
                headerApp = std::make_shared<HeaderApp>(this);
                headContainer = headerApp->getUIContainer();
                headContainer->setParent(screen);
                headContainer->setVisible(true);
                headContainer->setFit(Container::Fit::FILL, Container::Fit::OFF);
                if (centerContainer) {
                    centerContainer->setPosition(0, 30);
                    centerContainer->setDimensions(screen->getWidth(), screen->getHeight() - 30);
                }
            }
        }
    });
}

void AviTab::zoomIn() {
    // called from simDriver thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->onMouseWheel(1, 0, 0);
        }
    });
}

void AviTab::zoomOut() {
    // called from simDriver thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->onMouseWheel(-1, 0, 0);
        }
    });
}

void AviTab::recentre() {
    // called from simDriver thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->recentre();
        }
    });
}

void AviTab::panLeft() {
    // called from simDriver thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(-10, 0); // 10% leftwards
        }
    });
}

void AviTab::panRight() {
    // called from simDriver thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(10, 0); // 10% rightwards
        }
    });
}

void AviTab::panUp() {
    // called from simDriver thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(0, -10); // 10% upwards
        }
    });
}

void AviTab::panDown() {
    // called from simDriver thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(0, 10); // 10% downwards
        }
    });
}

void AviTab::finishInstall() {
    // create any user-modifiable files, only if they do not already exist
    try {
        auto mapConfigDir(getAvitabDataDir() /"online-maps");
        std::filesystem::create_directories(mapConfigDir);
        auto mapConfigPath(mapConfigDir /"mapconfig.json");
        (void)std::make_unique<JsonConfig>(mapConfigPath, defaultMapConfigJson());
    } catch (const std::exception &e) {
        // report to the log, but not totally fatal
        logger::error("Unable to create default mapconfig.json");
    }
}

void AviTab::createPanel() {
    auto cfgFile = getAirplanePath() /"AviTab.json";
    try {
        JsonConfig cfg(cfgFile);
        int left = cfg.getInt("/panel/left");
        int bottom = cfg.getInt("/panel/bottom");
        int width = cfg.getInt("/panel/width");
        int height = cfg.getInt("/panel/height");
        bool enable = false;
        bool disableCaptureWindow = false;
        bool aircraftManaged = false;
        hideHeader = false;
        try {
            enable = cfg.getBool("/panel/enabled");
        } catch (...) {
        }
        try {
            hideHeader = cfg.getBool("/panel/hide_header");
        } catch (...) {
        }
        try {
            disableCaptureWindow = cfg.getBool("/panel/disable_capture_window");
        } catch (...) {
        }
        try {
            aircraftManaged = cfg.getBool("/panel/aircraft_managed");
        } catch (...) {
        }

        UiDriverBase::PanelControlMode mode = aircraftManaged ? UiDriverBase::PanelControlMode::AIRCRAFT_MANAGED
                                        : (disableCaptureWindow ? UiDriverBase::PanelControlMode::COMMAND_ONLY
                                                                : UiDriverBase::PanelControlMode::CAPTURE_WINDOW);

        guiLib->createPanel(left, bottom, width, height, mode);
        if (enable) {
            simDriver->enableAndPowerPanel();
        }
    } catch (const std::exception &e) {
        logger::info("No panel config - window only mode");
        hideHeader = false;
        guiLib->hidePanel();
    }
}

void AviTab::createLayout() {
    // runs in GUI thread
    auto screen = guiLib->screen();
    screen->setOnResize([this] { this->onScreenResize(); });

    if (!hideHeader) {
        if (!headerApp) {
            headerApp = std::make_shared<HeaderApp>(this);
            headContainer = headerApp->getUIContainer();
            headContainer->setParent(screen);
            headContainer->setVisible(true);
        }
    }

    if (!appLauncher) {
        showAppLauncher();
    }
}

void AviTab::onScreenResize() {
    auto screen = guiLib->screen();
    int width = screen->getWidth();
    int height = screen->getHeight();

    if (headerApp) {
        headerApp->onScreenResize(width, height);
    }

    if (appLauncher) {
        if (hideHeader) {
            appLauncher->onScreenResize(width, height);
        } else {
            appLauncher->onScreenResize(width, height - 30);
        }
    }
}

void AviTab::showAppLauncher() {
    if (!appLauncher) {
        appLauncher = std::make_shared<AppLauncher>(this);;
    }
    appLauncher->show();
}

void AviTab::showApp(AppId id) {
    if (appLauncher) {
        guiLib->executeLater([this, id] () {
            appLauncher->showApp(id);
        });
    }
}

void AviTab::setIsInMenu(bool inMenu) {
    simDriver->setIsInMenu(inMenu);
}

std::shared_ptr<Container> AviTab::createGUIContainer() {
    auto screen = guiLib->screen();
    auto container = std::make_shared<Container>(screen);
    container->setVisible(false);
    if (hideHeader) {
        container->setPosition(0, 0);
        container->setDimensions(screen->getWidth(), screen->getHeight());
    } else {
        container->setPosition(0, 30);
        container->setDimensions(screen->getWidth(), screen->getHeight() - 30);
    }

    return container;
}

void AviTab::showGUIContainer(std::shared_ptr<Container> container) {
    if (centerContainer) {
        centerContainer->setVisible(false);
    }

    auto screen = guiLib->screen();
    centerContainer = container;
    centerContainer->setParent(screen);
    if (hideHeader) {
        centerContainer->setPosition(0, 0);
        centerContainer->setDimensions(screen->getWidth(), screen->getHeight());
    } else {
        centerContainer->setPosition(0, 30);
        centerContainer->setDimensions(screen->getWidth(), screen->getHeight() - 30);
    }
    centerContainer->setVisible(true);
}

void AviTab::setBrightness(float brightness) {
    guiLib->setBrightness(brightness);
}

float AviTab::getBrightness() {
    return guiLib->getBrightness();
}

std::shared_ptr<navdb::NavDatabase> AviTab::getNavDatabase() {
    return navManager->getNavDatabase();
}

void AviTab::executeLater(std::function<void()> func) {
    guiLib->executeLater(func);
}

std::filesystem::path AviTab::getAvitabInstallDir() {
    return simDriver->getProgramPath();
}

std::filesystem::path AviTab::getAvitabDataDir() {
    return simDriver->getDataRootPath();
}

std::filesystem::path AviTab::getFlightPlansPath() {
    return simDriver->getFlightPlansPath();
}

std::filesystem::path AviTab::getAirplanePath() {
    return simDriver->getAirplanePath();
}

std::vector<float> AviTab::getMagneticVariations(std::vector<world::Location> &locs) {
    return simDriver->getMagneticVariations(locs);
}

std::string AviTab::getMETARForAirport(const std::string &icao) {
    return simDriver->getMETARForAirport(icao);
}

std::string AviTab::getNearestAirportId() {
    return simDriver->getNearestAirportId();
}

int AviTab::getWeatherAtLocation(const world::Location &loc, const float &altitude, std::string &weather) {
    return simDriver->getWeatherAtLocation(loc, altitude, weather);
}

void AviTab::loadUserFixes(const std::filesystem::path &filename) {
    navManager->loadUserFixes(filename);
}

std::shared_ptr<apis::ChartService> AviTab::getChartService() {
    return chartService;
}

std::shared_ptr<navdb::Route> AviTab::getRoute() {
    return activeRoute;
}

void AviTab::setRoute(std::shared_ptr<navdb::Route> route) {
    activeRoute = route;
}

void AviTab::updateMapExports(float lat, float lon, int zoom, float vrange) {
    simDriver->updateMapExports(lat, lon, zoom, vrange);
}

AircraftID AviTab::getActiveAircraftCount() {
    return simDriver->getActiveAircraftCount();
}

world::Position AviTab::getAircraftPosition(AircraftID id) {
    return simDriver->getAircraftPosition(id);
}

unsigned int AviTab::getFramesPerSecond() {
    return simDriver->getFramesPerSecond();
}

unsigned int AviTab::getZuluTimeSeconds() {
    return simDriver->getZuluTimeSeconds();
}

unsigned int AviTab::getLocalTimeSeconds() {
    return simDriver->getLocalTimeSeconds();
}

std::shared_ptr<Settings> AviTab::getSettings() {
    return simDriver->getSettings();
}

void AviTab::onHomeButton() {
    showAppLauncher();
}

void AviTab::close() {
    logger::info("Closing tablet");
    simDriver->runInSimDriver([this] () {
        if (guiLib->hasNativeWindow()) {
            guiLib->pauseNativeWindow();
        }
    });
}

void AviTab::stopApp() {
    // This function is called by the simDriver
    // and the simDriver will never call the simDriver callback
    // again. If the GUI is currently waiting on an simDriver
    // job to run, we would create a deadlock now. So for a proper
    // shutdown, we must do the following:

    // remember the last window position
    auto rect = guiLib->getNativeWindowRect();
    simDriver->getSettings()->saveWindowRect(rect);

    // Cancel the loading if it is still running
    navManager->stop();

    // Stop the chart APIs so they no longer call the GUI
    chartService->stop();

    // Tell the GUI to not execute more background jobs
    // after the current ones have finished
    guiLib->signalStop();

    // Let the simDriver run its callbacks one last time,
    // letting the GUI jobs finish to release the wait on the
    // simDriver
    simDriver->pauseSimDriverJobs();

    // now that the GUI thread is guranteed to finish, we can
    // do the rest of the cleanup
    simDriver->destroyMenu();
    simDriver->destroyCommands();

    // this will also join the GUI thread
    guiLib->destroyNativeWindow();

    cleanupLayout();
}

void AviTab::cleanupLayout() {
    logger::verbose("Stopping AviTab");
    headContainer.reset();
    centerContainer.reset();
    headerApp.reset();
    appLauncher.reset();
}

void AviTab::handleClickCommand(bool down, bool drag) {
    // called from X-Plane thread, processed in the simDriver
    uiDriver->passLeftClick(down, drag);
}

void AviTab::handleWheelUpCommand() {
    // called from X-Plane thread, processed in the simDriver
    uiDriver->passWheel(1);
}

void AviTab::handleWheelDownCommand() {
    // called from X-Plane thread, processed in the simDriver
    uiDriver->passWheel(-1);
}

void AviTab::changeChartTab(bool next) {
    guiLib->executeLater([this, next] () {
        if (appLauncher) {
            appLauncher->changeChartTab(next);
        }
    });
}

AviTab::~AviTab() {
    // runs in simDriver thread, destroy by PluginStop
    logger::verbose("~AviTab");
}

} /* namespace avitab */

static const char *defaultMapConfigJson() {
    return R"x(
[
    {
        "name": "OpenTopoMap",
        "servers": [
            "a.tile.opentopomap.org",
            "b.tile.opentopomap.org",
            "c.tile.opentopomap.org"
        ],
        "protocol": "https",
        "copyright": "Map Data (c) OpenStreetMap, SRTM - Map Style (c) OpenTopoMap (CC-BY-SA)",
        "url": "{z}/{x}/{y}.png",
        "min_zoom_level": 1,
        "max_zoom_level": 17,
        "tile_width_px": 256,
        "tile_height_px": 256,
        "enabled": true
    },
    {
        "name": "OpenStreetMap",
        "servers": [
            "tile.openstreetmap.org"
        ],
        "protocol": "https",
        "copyright": "Map tiles (c) OpenStreetMap (ODbL)",
        "url": "{z}/{x}/{y}.png",
        "min_zoom_level": 1,
        "max_zoom_level": 17,
        "tile_width_px": 256,
        "tile_height_px": 256,
        "enabled": false,
        "comment": "https://wiki.openstreetmap.org/wiki/Raster_tile_providers"
    }
]
)x";
}

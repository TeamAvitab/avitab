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

// REFACTOR - class AviTab is a muddle of core and simulation environment.
// It needs reworking.

#include <climits>
#include <future>
#include <filesystem>
#include <memory>
#include "AviTabCore.h"
#include "apps/AppFunctions.h"
#include "GUIDriver.h"
#include "Environment.h"
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
    AviTab(std::shared_ptr<Environment> env, std::shared_ptr<GUIDriver> gui);
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
    std::shared_ptr<Environment> env;
    std::shared_ptr<GUIDriver> guiDriver;
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
std::unique_ptr<AviTabCore> AviTabCore::CreateAviTabCore(std::shared_ptr<Environment> env, std::shared_ptr<GUIDriver> gui) {
    return std::make_unique<AviTab>(env, gui);
}

AviTab::AviTab(std::shared_ptr<Environment> e, std::shared_ptr<GUIDriver> gd):
    env(e),
    guiDriver(gd)
{
    // runs in environment thread, called by XPluginEnable
    // NOTE order here is important. NAV db must be created before GUI is started.
    navManager = std::make_unique<navdb::NavDbManager>(env->getXpNavDataRootPath(), env->getMsfsNavDataRootPath());
    guiLib = std::make_shared<LVGLToolkit>(guiDriver);
    img::TTFStamper::setFontDirectory(env->getFontDirectory());
    chartService = std::make_shared<apis::ChartService>(env->getDataRootPath());
    env->resumeEnvironmentJobs();
}

void AviTab::startApp() {
    // runs in environment thread, called by XPluginEnable
    logger::verbose("Starting AviTab %s", AVITAB_VERSION_STR);

    finishInstall();

    env->createMenu("AviTab");
    env->createCommand("AviTab/toggle_tablet", "Toggle Tablet", [this] (CommandState s) { if (s == CommandState::START) toggleTablet(); });
    env->createCommand("AviTab/zoom_in", "Zoom In", [this] (CommandState s) { if (s == CommandState::START) zoomIn(); });
    env->createCommand("AviTab/zoom_out", "Zoom Out", [this] (CommandState s) { if (s == CommandState::START) zoomOut(); });
    env->createCommand("AviTab/recentre", "Recentre", [this] (CommandState s) { if (s == CommandState::START) recentre(); });
    env->createCommand("AviTab/pan_left", "Pan left", [this] (CommandState s) { if (s == CommandState::START) panLeft(); });
    env->createCommand("AviTab/pan_right", "Pan right", [this] (CommandState s) { if (s == CommandState::START) panRight(); });
    env->createCommand("AviTab/pan_up", "Pan up", [this] (CommandState s) { if (s == CommandState::START) panUp(); });
    env->createCommand("AviTab/pan_down", "Pan down", [this] (CommandState s) { if (s == CommandState::START) panDown(); });
    env->createCommand("AviTab/Home", "Home Button",[this] (CommandState s) { if (s == CommandState::START) onHomeButton(); });

    // App commands
    env->createCommand("AviTab/app_charts", "Charts App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::CHARTS); });
    env->createCommand("AviTab/app_airports", "Airports App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::AIRPORTS); });
    env->createCommand("AviTab/app_routes", "Routes App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::ROUTES); });
    env->createCommand("AviTab/app_maps", "Maps App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::MAPS); });
    env->createCommand("AviTab/app_plane_manual", "Plane Manual App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::PLANE_MANUAL); });
    env->createCommand("AviTab/app_notes", "Notes App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::NOTES); });
    env->createCommand("AviTab/app_navigraph", "Navigraph App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::NAVIGRAPH); });
    env->createCommand("AviTab/app_about", "About App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::ABOUT); });
    env->createCommand("AviTab/chart_tab_next", "Chart tab next", [this] (CommandState s) { if (s == CommandState::START) changeChartTab(true); });
    env->createCommand("AviTab/chart_tab_prev", "Chart tab previous", [this] (CommandState s) { if (s == CommandState::START) changeChartTab(false); });

    // Direct control from panel integrations
    env->createCommand("AviTab/click_left", "Left click", [this] (CommandState s) { handleClickCommand(s == CommandState::START, s == CommandState::CONTINUE); });
    env->createCommand("AviTab/wheel_up", "Wheel up", [this] (CommandState s) { if (s == CommandState::START) handleWheelUpCommand(); });
    env->createCommand("AviTab/wheel_down", "Wheel down", [this] (CommandState s) { if (s == CommandState::START) handleWheelDownCommand(); });

    // X-Plane menu items
    env->addMenuEntry("Toggle Tablet", [this] { toggleTablet(); });
    env->addMenuEntry("Reset Position", [this] { resetWindowPosition(); });

    guiLib->setMouseWheelCallback([this] (int dir, int x, int y) {
        if (appLauncher) {
            appLauncher->onMouseWheel(dir, x, y);
        }
    });
    createPanel();
    guiLib->executeLater(std::bind(&AviTab::createLayout, this));

    std::string userfixes_file = env->getSettings()->getGeneralSetting<std::string>("userfixes_file");
    navManager->loadUserFixes(userfixes_file);

}

void AviTab::toggleTablet() {
    // runs in environment thread, called by menu or command
    try {
        if (!guiLib->hasNativeWindow()) {
            logger::info("Showing tablet");
            // It's possible that the user closed the window with the close button.
            // Since we don't get any callback for this, it's possible that we didn't store the last window coordinates yet.
            // For that reason, the last known position is tried first.
            auto rect = guiLib->getNativeWindowRect();
            if (rect.valid && !resetWindowRect) {
                env->getSettings()->saveWindowRect(rect);
            } else {
                rect = env->getSettings()->getWindowRect();
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
    // runs in environment thread
    env->getSettings()->saveWindowRect({});
    if (guiLib->hasNativeWindow()) {
        guiLib->pauseNativeWindow();
    }
    resetWindowRect = true;
    toggleTablet();
    resetWindowRect = false;
}

void AviTab::onPlaneLoad() {
    // runs in environment thread
    // close on plane reload to reset the VR window position
    close();
    env->onAircraftReload();
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
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->onMouseWheel(1, 0, 0);
        }
    });
}

void AviTab::zoomOut() {
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->onMouseWheel(-1, 0, 0);
        }
    });
}

void AviTab::recentre() {
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->recentre();
        }
    });
}

void AviTab::panLeft() {
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(-10, 0); // 10% leftwards
        }
    });
}

void AviTab::panRight() {
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(10, 0); // 10% rightwards
        }
    });
}

void AviTab::panUp() {
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(0, -10); // 10% upwards
        }
    });
}

void AviTab::panDown() {
    // called from environment thread
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

        GUIDriver::PanelControlMode mode = aircraftManaged ? GUIDriver::PanelControlMode::AIRCRAFT_MANAGED
                                        : (disableCaptureWindow ? GUIDriver::PanelControlMode::COMMAND_ONLY
                                                                : GUIDriver::PanelControlMode::CAPTURE_WINDOW);

        guiLib->createPanel(left, bottom, width, height, mode);
        if (enable) {
            env->enableAndPowerPanel();
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
    env->setIsInMenu(inMenu);
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
    return env->getProgramPath();
}

std::filesystem::path AviTab::getAvitabDataDir() {
    return env->getDataRootPath();
}

std::filesystem::path AviTab::getFlightPlansPath() {
    return env->getFlightPlansPath();
}

std::filesystem::path AviTab::getAirplanePath() {
    return env->getAirplanePath();
}

std::vector<float> AviTab::getMagneticVariations(std::vector<world::Location> &locs) {
    return env->getMagneticVariations(locs);
}

std::string AviTab::getMETARForAirport(const std::string &icao) {
    return env->getMETARForAirport(icao);
}

std::string AviTab::getNearestAirportId() {
    return env->getNearestAirportId();
}

int AviTab::getWeatherAtLocation(const world::Location &loc, const float &altitude, std::string &weather) {
    return env->getWeatherAtLocation(loc, altitude, weather);
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
    env->updateMapExports(lat, lon, zoom, vrange);
}

AircraftID AviTab::getActiveAircraftCount() {
    return env->getActiveAircraftCount();
}

world::Position AviTab::getAircraftPosition(AircraftID id) {
    return env->getAircraftPosition(id);
}

unsigned int AviTab::getFramesPerSecond() {
    return env->getFramesPerSecond();
}

unsigned int AviTab::getZuluTimeSeconds() {
    return env->getZuluTimeSeconds();
}

unsigned int AviTab::getLocalTimeSeconds() {
    return env->getLocalTimeSeconds();
}

std::shared_ptr<Settings> AviTab::getSettings() {
    return env->getSettings();
}

void AviTab::onHomeButton() {
    showAppLauncher();
}

void AviTab::close() {
    logger::info("Closing tablet");
    env->runInEnvironment([this] () {
        if (guiLib->hasNativeWindow()) {
            guiLib->pauseNativeWindow();
        }
    });
}

void AviTab::stopApp() {
    // This function is called by the environment
    // and the environment will never call the environment callback
    // again. If the GUI is currently waiting on an environment
    // job to run, we would create a deadlock now. So for a proper
    // shutdown, we must do the following:

    // remember the last window position
    auto rect = guiLib->getNativeWindowRect();
    env->getSettings()->saveWindowRect(rect);

    // Cancel the loading if it is still running
    navManager->stop();

    // Stop the chart APIs so they no longer call the GUI
    chartService->stop();

    // Tell the GUI to not execute more background jobs
    // after the current ones have finished
    guiLib->signalStop();

    // Let the environment run its callbacks one last time,
    // letting the GUI jobs finish to release the wait on the
    // environment
    env->pauseEnvironmentJobs();

    // now that the GUI thread is guranteed to finish, we can
    // do the rest of the cleanup
    env->destroyMenu();
    env->destroyCommands();

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
    // called from X-Plane thread, processed in the sim environment
    guiDriver->passLeftClick(down, drag);
}

void AviTab::handleWheelUpCommand() {
    // called from X-Plane thread, processed in the sim environment
    guiDriver->passWheel(1);
}

void AviTab::handleWheelDownCommand() {
    // called from X-Plane thread, processed in the sim environment
    guiDriver->passWheel(-1);
}

void AviTab::changeChartTab(bool next) {
    guiLib->executeLater([this, next] () {
        if (appLauncher) {
            appLauncher->changeChartTab(next);
        }
    });
}

AviTab::~AviTab() {
    // runs in environment thread, destroy by PluginStop
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

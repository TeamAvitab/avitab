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

#include <future>
#include <filesystem>
#include <memory>
#include "AviTabCore.h"
#include "apps/AppFunctions.h"
#include "UiDriver.h"
#include "SimDriver.h"
#include "platform/Platform.h"
#include "platform/CrashHandler.h"
#include "Logger.h"
#include "JsonConfig.h"
#include "gui/LVGLToolkit.h"
#include "image/TTFStamper.h"
#include "apps/HeaderApp.h"
#include "apps/AppLauncher.h"
#include "nav/NavDbManager.h"
#include "nav/routing/RouteFinder.h"
#include "AviTabBuildSettings.h"

namespace avitab {

class AviTab : public AviTabCore, public AppFunctions {
public:
    AviTab();
    ~AviTab();

public:
    // ----------------------------
    // Implementation of AviTabCore
    // ----------------------------

    // APIs called by the product wrappers.
    // The top-level state machine is start -> enable -> disable { -> enable -> disable } -> stop
    // Start/stop are moore-or-less equivalent to construction/destruction
    // Enable/disable correspond to the X-Plane plugin events and can be iterated. Other products just call
    // enable once after start, and disable once before stop.
    void start(std::shared_ptr<Settings> prefs, std::shared_ptr<SimDriverBase> sim, std::shared_ptr<UiDriverBase> gui) override;
    void enable() override;
    void disable() override;
    void stop() override;

    // API to adapt apps when plane changes
    void onPlaneLoad(bool hideHeaderApp) override;

    // APIs for injection of commands from the product, mainly from the X-Plane plugin
    void commandShowApp(const std::string &appName) override;
    void commandZoomIn() override;
    void commandZoomOut() override;
    void commandRecentre() override;
    void commandPanLeft() override;
    void commandPanRight() override;
    void commandPanUp() override;
    void commandPanDown() override;
    void commandTabNext() override;
    void commandTabPrevious() override;
    void commandWheelUp(int x, int y) override;
    void commandWheelDown(int x, int y) override;

public:
    // ------------------------------
    // Implementation of AppFunctions
    // ------------------------------

    std::filesystem::path getAvitabInstallDir() override;
    std::filesystem::path getAvitabDataDir() override;
    std::filesystem::path getAirplanePath() override;
    std::filesystem::path getFlightPlansPath() override;

    std::shared_ptr<Settings> getSettings() override;
    std::shared_ptr<apis::ChartService> getChartService() override;
    std::shared_ptr<navdb::NavDatabase> getNavDatabase() override;

    void onHomeButtonPressed() override;
    void onAppClosed() override;
    void notifyHomeScreenActive(bool inMenu) override;
    void notifyMapConfiguration(float lat, float lon, int zoom, float vrange) override;

    void loadUserFixes(const std::filesystem::path &filename) override;
    std::string getNearestAirportId() override;
    std::vector<float> getMagneticVariations(std::vector<world::Location> &locs) override;
    std::shared_ptr<navdb::Route> getRoute() override;
    void setRoute(std::shared_ptr<navdb::Route> route) override;

    std::string getMETARForAirport(const std::string &icao) override;
    int getWeatherAtLocation(const world::Location &loc, const float &altitude, std::string &weather) override;

    AircraftID getActiveAircraftCount() override;
    world::Position getAircraftPosition(AircraftID id) override;

    unsigned int getFramesPerSecond() override;
    unsigned int getZuluTimeSeconds() override;
    unsigned int getLocalTimeSeconds() override;

    void setBrightness(float brightness) override;
    float getBrightness() override;

    void executeLater(std::function<void()> func) override;

    std::shared_ptr<Container> createGUIContainer() override;
    void showGUIContainer(std::shared_ptr<Container> container) override;

private:
    void finishInstall();

    void buildHomeScreen();
    void teardownGUI();

    void showAppLauncher();
    void showApp(AppId id);

    void onScreenResize();

    void coreThreadLoop();

private:
    std::shared_ptr<Settings> settings;
    std::shared_ptr<SimDriverBase> simDriver;
    std::shared_ptr<UiDriverBase> uiDriver;
    std::unique_ptr<navdb::NavDbManager> navManager;
    std::shared_ptr<navdb::Route> activeRoute;
    std::shared_ptr<apis::ChartService> chartService;
    std::shared_ptr<LVGLToolkit> lvglUI;

    std::unique_ptr<std::thread> coreThread;
    std::atomic_bool coreThreadActive;
    std::atomic_bool coreEnabled;
    std::recursive_mutex pendingTasksMutex;
    std::vector<std::function<void()> > pendingTasks;

    bool hideHeaderApp = false;
    std::shared_ptr<Label> loadLabel;
    std::shared_ptr<Container> headContainer;
    std::shared_ptr<Container> centerContainer;

    std::shared_ptr<App> headerApp;
    std::shared_ptr<AppLauncher> appLauncher;
};

static const char *defaultMapConfigJson();

// Factory
std::shared_ptr<AviTabCore> AviTabCore::CreateAviTabCore() {
    return std::make_shared<AviTab>();
}

AviTab::AviTab() {
}

AviTab::~AviTab() {
    // runs in simDriver thread, destroy by PluginStop
    logger::verbose("~AviTab");
}

void AviTab::start(std::shared_ptr<Settings> prefs, std::shared_ptr<SimDriverBase> sim, std::shared_ptr<UiDriverBase> gui) {
    // runs in product main thread (for X-Plane plugin called by XPluginStart).
    // will only be called once per simulation run.
    // initialise all the background services, but don't enable them yet.
    // enable() should (re)start UI loop, disable() should pause UI loop.
    // stop() will be called once before product exits to clean up.
    logger::verbose("Starting AviTab %s", AVITAB_VERSION_STR);

    settings = prefs;
    simDriver = sim;
    uiDriver = gui;

    finishInstall();

    navManager = std::make_unique<navdb::NavDbManager>(simDriver->getXpNavDataRootPath(), simDriver->getMsfsNavDataRootPath());
    img::TTFStamper::setFontDirectory(simDriver->getFontDirectory());
    std::vector<std::string> remote_georefs_urls = settings->getGeneralSetting<std::vector<std::string>>("remote_georefs_urls");
    chartService = std::make_shared<apis::ChartService>(simDriver->getDataRootPath(), remote_georefs_urls);

    std::string userfixes_file = settings->getGeneralSetting<std::string>("userfixes_file");
    navManager->loadUserFixes(userfixes_file);

    lvglUI = std::make_shared<LVGLToolkit>(uiDriver);

    uiDriver->setResizeCallback([this] (int w, int h) {
        executeLater([this, w, h] {
            lvglUI->resizeScreen(w, h);
        });
    });

    coreEnabled = false;
    coreThreadActive = true;
    coreThread = std::make_unique<std::thread>(&AviTab::coreThreadLoop, this);
}

void AviTab::enable() {
    // runs in product main thread (for X-Plane plugin called by XPluginEnable).
    logger::verbose("Enabling AviTab core");

    coreEnabled = true;
    executeLater(std::bind(&AviTab::buildHomeScreen, this));
    executeLater(std::bind(&AviTab::showAppLauncher, this));
}

void AviTab::disable() {
    // runs in product main thread (for X-Plane plugin called by XPluginDisable).
    logger::verbose("Disabling AviTab core");

    executeLater(std::bind(&AviTab::teardownGUI, this));
    // wait until the GUI has been torn down
    while (1) {
        auto f0 = lvglUI->getFrameCount();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto f1 = lvglUI->getFrameCount();
        if (f0 == f1) break;
    }
    coreEnabled = false;
}

void AviTab::stop() {
    // runs in product main thread (for X-Plane plugin called by XPluginStop).
    // will only be called once per simulation run.
    // stop() will be called once before product exits to clean up.

    // this will also join the GUI thread
    if (coreThread) {
        coreThreadActive = false;
        coreThread->join();
        coreThread.reset();
    }

    // Stop the chart APIs so they no longer call the GUI
    chartService->stop();

    // Stop the NAV amanger service (cancel the loading if it is still running).
    navManager->stop();

    uiDriver.reset();
    simDriver.reset();
    settings.reset();
}

void AviTab::onPlaneLoad(bool noHeader) {
    executeLater([this] () {
        if (appLauncher) {
            appLauncher->onPlaneLoad();
        }
    });
    executeLater([this, noHeader] () {
        auto screen = lvglUI->screen();
        hideHeaderApp = noHeader;
        if (hideHeaderApp) {
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

void AviTab::commandShowApp(const std::string &appName) {
    if (appName.empty()) {
        showAppLauncher();
    } else if (appName == "charts") {
        showApp(AppId::CHARTS);
    } else if (appName == "airports") {
        showApp(AppId::AIRPORTS);
    } else if (appName == "routes") {
        showApp(AppId::ROUTES);
    } else if (appName == "maps") {
        showApp(AppId::MAPS);
    } else if (appName == "manuals") {
        showApp(AppId::MANUALS);
    } else if (appName == "notes") {
        showApp(AppId::NOTES);
    } else if (appName == "providers") {
        showApp(AppId::PROVIDERS);
    } else if (appName == "about") {
        showApp(AppId::ABOUT);
    }
}

void AviTab::commandZoomIn() {
    // called from simDriver thread
    executeLater([this] () {
        if (appLauncher) {
            appLauncher->zoom(true);
        }
    });
}

void AviTab::commandZoomOut() {
    // called from simDriver thread
    executeLater([this] () {
        if (appLauncher) {
            appLauncher->zoom(false);
        }
    });
}

void AviTab::commandRecentre() {
    // called from simDriver thread
    executeLater([this] () {
        if (appLauncher) {
            appLauncher->recentre();
        }
    });
}

void AviTab::commandPanLeft() {
    // called from simDriver thread
    executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(-10, 0); // 10% leftwards
        }
    });
}

void AviTab::commandPanRight() {
    // called from simDriver thread
    executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(10, 0); // 10% rightwards
        }
    });
}

void AviTab::commandPanUp() {
    // called from simDriver thread
    executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(0, -10); // 10% upwards
        }
    });
}

void AviTab::commandPanDown() {
    // called from simDriver thread
    executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(0, 10); // 10% downwards
        }
    });
}

void AviTab::commandTabNext() {
    executeLater([this] () {
        if (appLauncher) {
            appLauncher->changeChartTab(true);
        }
    });
}

void AviTab::commandTabPrevious() {
    executeLater([this] () {
        if (appLauncher) {
            appLauncher->changeChartTab(false);
        }
    });
}

void AviTab::commandWheelUp(int x, int y) {
    // called from the sim driver
    executeLater([this, x, y] () {
        if (appLauncher) {
            appLauncher->onMouseWheel(1, x, y);
        }
    });
}

void AviTab::commandWheelDown(int x, int y) {
    // called from the sim driver
    executeLater([this, x, y] () {
        if (appLauncher) {
            appLauncher->onMouseWheel(-1, x, y);
        }
    });
}

std::filesystem::path AviTab::getAvitabInstallDir() {
    return simDriver->getProgramPath();
}

std::filesystem::path AviTab::getAvitabDataDir() {
    return simDriver->getDataRootPath();
}

std::filesystem::path AviTab::getAirplanePath() {
    return simDriver->getAirplanePath();
}

std::filesystem::path AviTab::getFlightPlansPath() {
    return simDriver->getFlightPlansPath();
}

std::shared_ptr<Settings> AviTab::getSettings() {
    return settings;
}

std::shared_ptr<apis::ChartService> AviTab::getChartService() {
    return chartService;
}

std::shared_ptr<navdb::NavDatabase> AviTab::getNavDatabase() {
    return navManager->getNavDatabase();
}

void AviTab::onHomeButtonPressed() {
    showAppLauncher();
}

void AviTab::onAppClosed() {
    uiDriver->requestCloseWindow();
}

void AviTab::notifyHomeScreenActive(bool inMenu) {
    simDriver->notifyIsInMenu(inMenu);
}

void AviTab::notifyMapConfiguration(float lat, float lon, int zoom, float vrange) {
    simDriver->notifyMapConfiguration(lat, lon, zoom, vrange);
}

void AviTab::loadUserFixes(const std::filesystem::path &filename) {
    navManager->loadUserFixes(filename);
}

std::string AviTab::getNearestAirportId() {
    return simDriver->getNearestAirportId();
}

std::vector<float> AviTab::getMagneticVariations(std::vector<world::Location> &locs) {
    return simDriver->getMagneticVariations(locs);
}

void AviTab::setRoute(std::shared_ptr<navdb::Route> route) {
    activeRoute = route;
}

std::shared_ptr<navdb::Route> AviTab::getRoute() {
    return activeRoute;
}

std::string AviTab::getMETARForAirport(const std::string &icao) {
    return simDriver->getMETARForAirport(icao);
}

int AviTab::getWeatherAtLocation(const world::Location &loc, const float &altitude, std::string &weather) {
    return simDriver->getWeatherAtLocation(loc, altitude, weather);
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

void AviTab::setBrightness(float brightness) {
    lvglUI->setBrightness(brightness);
}

float AviTab::getBrightness() {
    return lvglUI->getBrightness();
}

void AviTab::executeLater(std::function<void()> func) {
    std::lock_guard<std::recursive_mutex> lock(pendingTasksMutex);
    if (coreThreadActive && coreEnabled) {
        pendingTasks.push_back(func);
    }
}

std::shared_ptr<Container> AviTab::createGUIContainer() {
    auto screen = lvglUI->screen();
    auto container = std::make_shared<Container>(screen);
    container->setVisible(false);
    if (hideHeaderApp) {
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

    auto screen = lvglUI->screen();
    centerContainer = container;
    centerContainer->setParent(screen);
    if (hideHeaderApp) {
        centerContainer->setPosition(0, 0);
        centerContainer->setDimensions(screen->getWidth(), screen->getHeight());
    } else {
        centerContainer->setPosition(0, 30);
        centerContainer->setDimensions(screen->getWidth(), screen->getHeight() - 30);
    }
    centerContainer->setVisible(true);
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

void AviTab::buildHomeScreen() {
    // runs in core thread
    auto screen = lvglUI->screen();
    screen->setOnResize([this] { this->onScreenResize(); });

    if (!hideHeaderApp) {
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

void AviTab::teardownGUI() {
    headContainer.reset();
    centerContainer.reset();
    headerApp.reset();
    appLauncher.reset();
}

void AviTab::showAppLauncher() {
    if (!appLauncher) {
        appLauncher = std::make_shared<AppLauncher>(this);;
    }
    appLauncher->show();
}

void AviTab::showApp(AppId id) {
    if (appLauncher) {
        executeLater([this, id] () {
            appLauncher->showApp(id);
        });
    }
}

void AviTab::onScreenResize() {
    auto screen = lvglUI->screen();
    int width = screen->getWidth();
    int height = screen->getHeight();

    if (headerApp) {
        headerApp->onScreenResize(width, height);
    }

    if (appLauncher) {
        if (hideHeaderApp) {
            appLauncher->onScreenResize(width, height);
        } else {
            appLauncher->onScreenResize(width, height - 30);
        }
    }
}

void AviTab::coreThreadLoop() {
    using namespace std::chrono_literals;
    crash::ThreadCookie crashCookie;

    logger::verbose("Core thread has id %d", std::this_thread::get_id());

    while (coreThreadActive) {
        if (coreEnabled) {
            // chrono clocks run at 64Hz precision in mingw, use something custom
            auto startAt = platform::measureTime();

            try {
                // first run the actual GUI tasks, i.e. let LVGL do its animations etc.
                // this also includes LVGL retrieving mouse button events and position
                lvglUI->doLVGLtasks();

                // then run our own tasks
                // grab the mutex briefly to extract the tasks into a local list
                // this avoids potential race and deadlock conditions.
                std::vector<std::function<void()>> tasks;
                {
                    std::lock_guard<std::recursive_mutex> lock(pendingTasksMutex);
                    tasks = pendingTasks;
                    pendingTasks.clear();
                }

                for (auto &task: tasks) {
                    task();
                }

                // NOTE - This is a bit of a hack, but it works for now. The problem is that
                // LVGLv6 does not have a proper mouse wheel event, so we have to fake it by
                // reading the wheel clicks and mouse position from the driver and then passing
                // this through to the active app.
                auto clicks = uiDriver->getWheelClicks();
                if (clicks && appLauncher) {
                    bool ignored;
                    int x, y;
                    uiDriver->readPointerState(x, y, ignored);
                    appLauncher->onMouseWheel(clicks, x, y);
                }

                // finally handle any keyboard events (including LVGL on-screen keyboard)
                lvglUI->handleKeyboard();
            } catch (const std::exception &e) {
                logger::error("Exception in GUI: %s", e.what());
            }

            std::this_thread::sleep_for(1ms);
            auto elapsedMillis = platform::getElapsedMillis(startAt);
            lvglUI->doLVGLtick(elapsedMillis);
        } else {
            std::this_thread::sleep_for(100ms);
        }
    }

    logger::verbose("Core thread destroyed");
}

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

} /* namespace avitab */

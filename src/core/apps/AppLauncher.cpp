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
#include "Logger.h"
#include "AppLauncher.h"
#include "ChartsApp.h"
#include "NotesApp.h"
#include "About.h"
#include "PlaneManualApp.h"
#include "AirportApp.h"
#include "RouteApp.h"
#include "MapApp.h"
#include "ProvidersApp.h"

namespace avitab {

AppLauncher::AppLauncher(FuncsPtr appFuncs):
    App(appFuncs)
{
    auto theme = getUITheme();
    auto cont = getUIContainer();
    cont->setLayoutFlex();
    cont->setLocalStyle("uiContainer");
    auto iconDir = api().getAvitabThemeDir()/theme->currentName();

    addEntry<ChartsApp>("Charts", iconDir / Theme::Icons::Charts, AppId::CHARTS);
    addEntry<AirportApp>("Airports", iconDir / Theme::Icons::Airports, AppId::AIRPORTS);
    addEntry<RouteApp>("Routes", iconDir / Theme::Icons::Routes, AppId::ROUTES);
    addEntry<MapApp>("Maps", iconDir / Theme::Icons::Maps, AppId::MAPS);
    addEntry<PlaneManualApp>("Aircraft", iconDir / Theme::Icons::PlaneManual, AppId::PLANE_MANUAL);
    addEntry<NotesApp>("Notes", iconDir / Theme::Icons::Notes, AppId::NOTES);

    if (api().getChartService()->getNavigraph()->isSupported() || api().getChartService()->getChartFox()->isSupported()) {
        addEntry<ProvidersApp>("Providers", iconDir / Theme::Icons::Providers, AppId::NAVIGRAPH);
    }

    addEntry<About>("About", iconDir / Theme::Icons::About, AppId::ABOUT);
}

void AppLauncher::onScreenResize(int width, int height) {
    // FIXME check IsInMenu ?
    auto cont = getUIContainer();
    cont->setHeight(height);
    for (auto &entry: entries) {
        entry.app->onScreenResize(width, height);
    }
}

void AppLauncher::show() {
    if (activeApp) {
        activeApp->suspend();
    }
    App::show();
    api().setIsInMenu(true);
}

void AppLauncher::showApp(AppId id) {
    for (auto &entry: entries) {
        if (entry.id == id) {
            if (activeApp) {
                activeApp->suspend();
            }
            activeApp = entry.app;
            activeApp->resume();
            activeApp->show();
            api().setIsInMenu(false);
            break;
        }
    }
}

template<typename T>
void AppLauncher::addEntry(const std::string& name, const std::filesystem::path& icon, AppId id) {
    auto app = startSubApp<T>();
    app->setOnExit([this] () {
        this->show();
    });

    Entry entry;
    entry.id = id;
    entry.app = std::move(app);
    entry.button = std::make_shared<Button>(getUIContainer(), icon, name, 100);
    entry.button->setLocalStyle("appButton");
    entries.push_back(entry);

    size_t index = entries.size() - 1;
    entry.button->setCallback([this, index] (const Button &) {
        showApp(entries[index].id);
    });
}

void AppLauncher::onPlaneLoad() {
    for (auto &entry: entries) {
        entry.app->onPlaneLoad();
    }
}

void AppLauncher::onMouseWheel(int dir, int x, int y) {
    if (activeApp) {
        activeApp->onMouseWheel(dir, x, y);
    }
}
void AppLauncher::changeChartTab(bool next) {
    if (activeApp) {
        activeApp->changeChartTab(next);
    }
}

void AppLauncher::recentre() {
    if (activeApp) {
        activeApp->recentre();
    }
}

void AppLauncher::pan(int x, int y) {
    if (activeApp) {
        activeApp->pan(x, y);
    }
}


} /* namespace avitab */

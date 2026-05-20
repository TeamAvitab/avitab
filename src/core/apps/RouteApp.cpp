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
#include <sstream>
#include <iomanip>
#include "RouteApp.h"
#include "nav/routing/RouteFinder.h"
#include "nav/loaders/xplane/FMSLoader.h"
#include "Logger.h"

namespace avitab {

RouteApp::RouteApp(FuncsPtr appFuncs):
    App(appFuncs)
{
    auto ui = getUIContainer();
    window = std::make_shared<Window>(ui, "Route");
    window->setOnClose([this] () {
        showDeparturePage();
        exit();
    });

    chooserContainer = std::make_shared<Container>();
    chooserContainer->setDimensions(ui->getWidth() * 0.75, ui->getHeight() * 0.75);
    chooserContainer->centerInParent();
    chooserContainer->setVisible(false);

    showDeparturePage();
}

void RouteApp::showDeparturePage() {
    reset();

    window->setCaption("Route Wizard - Departure");

    loadContainer = std::make_shared<Container>(window);
    loadButton = std::make_shared<Button>(loadContainer, "Load");
    loadContainer->setDimensions(window->getWidth(), loadButton->getHeight() + 2);
    loadButton->alignInTopRight();
    loadButton->setCallback([this] (const Button &) { selectFlightPlanFile(); });

    label = std::make_shared<Label>(window, "Departure airport code:");
    label->alignBelow(loadContainer);

    departureField = std::make_shared<TextArea>(window, "");
    departureField->setMultiLine(false);
    departureField->alignBelow(label);

    checkBox = std::make_shared<Checkbox>(window, "Upper airways");
    checkBox->alignBelow(departureField);

    keys = std::make_shared<Keyboard>(window, departureField);
    keys->hideEnterKey();

    keys->setOnCancel([this] () {
        departureField->setText("");
    });

    keys->setOnOk([this] () { api().executeLater([this] () {
        if (checkBox->isChecked()) {
            airwayLevel = navdb::AirwayLevel::UPPER;
        } else {
            airwayLevel = navdb::AirwayLevel::LOWER;
        }
        onDepartureEntered(departureField->getText());
     });});

    keys->setPosition(0, window->getContentHeight() - keys->getHeight());
}

void RouteApp::onDepartureEntered(const std::string& departure) {
    auto navWorld = api().getNavDatabase();

    auto ap = navWorld->findAirportByID(departure);
    if (!ap) {
        showError("Airport not found");
        return;
    }

    departureNode = ap;

    showArrivalPage();
}

void RouteApp::showArrivalPage() {
    reset();

    window->setCaption("Route Wizard - Arrival");

    label = std::make_shared<Label>(window, "Arrival airport code:");
    label->alignInTopLeft();

    arrivalField = std::make_shared<TextArea>(window, "");
    arrivalField->setMultiLine(false);
    arrivalField->alignBelow(label);

    keys = std::make_shared<Keyboard>(window, arrivalField);
    keys->hideEnterKey();

    keys->setOnCancel([this] () {
        arrivalField->setText("");
    });

    keys->setOnOk([this] () { api().executeLater([this] () {
        onArrivalEntered(arrivalField->getText());
    });});

    keys->setPosition(-5, window->getContentHeight() - keys->getHeight());
}

void RouteApp::onArrivalEntered(const std::string& arrival) {
    if (arrival == "") {
        return;
    }
    auto navWorld = api().getNavDatabase();

    auto ap = navWorld->findAirportByID(arrival);
    if (!ap) {
        showError("Airport not found");
        return;
    }

    if (ap->getIdent() == departureNode->getIdent()) {
        showError("Arrival must be different from departure");
        arrivalField->setText("");
        return;
    }

    arrivalNode = ap;

    showStatus("Searching for route");
    auto f = std::async(std::launch::async, [this] {
        return asyncSearchForRoute();
    });
    auto r = f.get();
    if (r) {
        api().setRoute(r);
        fmsRawText = "";
        showRoute();
    }
}

std::shared_ptr<navdb::Route> RouteApp::asyncSearchForRoute() {
    std::shared_ptr<navdb::Route> r;
    try {
        auto router = std::make_unique<navdb::RouteFinder>(api().getNavDatabase(), departureNode, arrivalNode, airwayLevel);
        r = router->find();
        std::vector<world::Location> wpts;
        r->iterateLegs([&wpts] (const navdb::Route::RouteNode n, const navdb::Route::RouteLeg, const navdb::Route::RouteNode, float, float, float) {
            wpts.push_back(n->getLocation());
        });
        auto magVars = api().getMagneticVariations(wpts);
        r->applyMagDecls(magVars);
    } catch (const std::exception &e) {
        std::string error = std::string("Couldn't find a route: ") + e.what();
        showError(error);
    }
    return r;
}


void RouteApp::showRoute() {
    reset();

    auto route = api().getRoute();

    std::stringstream desc;
    desc << std::fixed << std::setprecision(0);

    std::string shortRoute;
    if (fmsRawText.size()) {
        shortRoute = fmsRawText;
    } else {
        shortRoute = toShortRouteDescription();
    }

    desc << "Route: \n";
    desc << shortRoute << "\n";

    double directKm = route->getDirectDistance() / 1000;
    double routeKm = route->getRouteDistance() / 1000;
    double directNm = directKm * world::KM_TO_NM;
    double routeNm = routeKm * world::KM_TO_NM;

    desc << "-----\n";
    std::string detailedRoute = toDetailedRouteDescription();
    desc << detailedRoute << "\n";
    desc << "-----\n";
    desc << "Direct distance: " << directKm << "km / " << directNm << "nm\n";
    desc << "Route distance: " << routeKm << "km / " << routeNm << "nm\n";

    label = std::make_shared<Label>(window, "");
    label->setAllowColors(true);
    label->setLongMode(true);
    label->setText(desc.str());
    label->setDimensions(window->getContentWidth(), window->getHeight() - 40);
    label->alignBelow(loadContainer);
}

void RouteApp::reset() {
    checkBox.reset();
    errorMessage.reset();
    statusMessage.reset();
    list.reset();
    label.reset();
    departureField.reset();
    arrivalField.reset();
    keys.reset();
    nextButton.reset();
    cancelButton.reset();
    loadButton.reset();
}

void RouteApp::showError(const std::string& msg) {
    errorMessage = std::make_shared<MessageBox>(getUIContainer(), msg);
    errorMessage->addButton("Ok", [this] () {
        api().executeLater([this] () {
            errorMessage.reset();
        });
    });
    errorMessage->centerInParent();
}

void RouteApp::showStatus(const std::string& msg) {
    statusMessage = std::make_shared<MessageBox>(getUIContainer(), msg);
    statusMessage->centerInParent();
}

std::string RouteApp::toShortRouteDescription() {
    std::stringstream desc;

    auto route = api().getRoute();
    route->iterateRouteShort([this, &desc] (const std::shared_ptr<navdb::NodeLink> via, const std::shared_ptr<navdb::Node> to) {
        if (via) {
            if (!via->isProcedure()) {
                desc << " #368BC1 " << via->getIdent() << "#";
            }
        }

        if (to) {
            if (to == departureNode || to == arrivalNode) {
                desc << " #99CC00";
            }

            desc << " " << to->getIdent();

            if (to == departureNode || to == arrivalNode) {
                desc << "#";
            }
        }
    });

    return desc.str();
}
std::string RouteApp::toDetailedRouteDescription() {
    std::stringstream desc;

    auto route = api().getRoute();
    route->iterateLegs([this, &desc] (
            const std::shared_ptr<navdb::Node> from,
            const std::shared_ptr<navdb::NodeLink> via,
            const std::shared_ptr<navdb::Node> to,
            float distanceNm,
            float initialTrueBearing,
            float initialMagneticBearing) {

        std::string from_str = from ? from->getIdent() : "(no from)";
        std::string via_str = via ? via->getIdent() : "-";
        std::string to_str = to ? to->getIdent() : "(no to)";
        int showInitialTrueBearing = (int)(initialTrueBearing + 0.5) % 360;
        int showInitialMagneticBearing = (int)(initialMagneticBearing + 0.5) % 360;

        desc << from_str.c_str() << "\n" << "    " << via_str.c_str() << "  " <<
            std::setfill('0') << std::setw(3) << showInitialTrueBearing << "°T" << "/" <<
            std::setfill('0') << std::setw(3) << showInitialMagneticBearing << "°M" <<
            "  " << (int)distanceNm << "nm\n";

        if (to == arrivalNode) {
            desc << to_str.c_str() << "\n";
        }
    });

    return desc.str();
}

void RouteApp::selectFlightPlanFile() {
    reset();
    fileChooser = std::make_unique<FileChooser>(&api(), "Flight Plan: ", api().getFlightPlansPath());
    fileChooser->setFilterRegex("\\.fms$");
    fileChooser->setSelectCallback([this] (const std::filesystem::path &selectedUTF8) {
        api().executeLater([this, selectedUTF8] () {
            try {
                fileChooser.reset();
                chooserContainer->setVisible(false);
                loadRouteFromFMS(selectedUTF8);
                showRoute();
            } catch (const std::exception &e) {
                showDeparturePage();
                logger::warn("Couldn't load FMS file '%s': %s", selectedUTF8.c_str(), e.what());
                showError("Couldn't load FMS file, see Avitab.log");
                return;
            }
        });
    });
    fileChooser->setCancelCallback([this] () {
        api().executeLater([this] () {
            fileChooser.reset();
            chooserContainer->setVisible(false);
            showDeparturePage();
        });
    });
    fileChooser->show(chooserContainer);
    chooserContainer->setVisible(true);
}

void RouteApp::loadRouteFromFMS(const std::filesystem::path &fmsFilename)
{
    // load the raw text from the file
    std::ifstream ifs(fmsFilename);
    if (!ifs) {
        throw std::runtime_error(std::string("Couldn't read FMS file ") + fmsFilename.string().c_str());
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    fmsRawText = ss.str();

    // now use the FMS parser to extract a list of waypoints
    auto navdb = api().getNavDatabase();
    xdata::FMSLoader loader(navdb.get());
    auto route = loader.load(fmsFilename); // throws exception if route has no legs
    logger::info("Loaded route with %d waypoints from %s", route->size(), fmsFilename.string().c_str());

    // Getting magVar from XPlane involves thread context-switch, so process all waypoints together
    std::vector<world::Location> wpts;
    route->iterateLegs([&wpts] (const navdb::Route::RouteNode n, const navdb::Route::RouteLeg, const navdb::Route::RouteNode, float, float, float) {
        wpts.push_back(n->getLocation());
    });
    auto magVars = api().getMagneticVariations(wpts);
    route->applyMagDecls(magVars);

    departureNode = route->front();
    arrivalNode = route->back();

    api().setRoute(route);
}

} /* namespace avitab */

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
#include <set>
#include "App.h"
#include "nav/routing/Route.h"
#include "apps/components/FileChooser.h"
#include "gui/widgets/TabGroup.h"
#include "gui/widgets/Page.h"
#include "gui/widgets/TextArea.h"
#include "gui/widgets/Keyboard.h"
#include "gui/widgets/Label.h"
#include "gui/widgets/Button.h"
#include "gui/widgets/Checkbox.h"
#include "gui/widgets/MessageBox.h"
#include "gui/widgets/Window.h"
#include "gui/widgets/DropDownList.h"

namespace avitab {

class RouteApp: public App {
public:
    RouteApp(FuncsPtr appFuncs);
private:
    std::shared_ptr<Window> window;
    std::shared_ptr<Label> label;
    std::shared_ptr<TextArea> departureField, arrivalField;
    std::shared_ptr<Container> loadContainer, chooserContainer;
    std::shared_ptr<Button> loadButton;
    std::shared_ptr<Keyboard> keys;
    std::shared_ptr<DropDownList> list;
    std::shared_ptr<MessageBox> errorMessage;
    std::shared_ptr<MessageBox> statusMessage;
    std::shared_ptr<Button> nextButton, cancelButton;
    std::shared_ptr<Checkbox> checkBox;
    std::unique_ptr<FileChooser> fileChooser;

    navdb::AirwayLevel airwayLevel = navdb::AirwayLevel::UPPER;
    std::shared_ptr<navdb::Node> departureNode, arrivalNode;
    std::shared_ptr<navdb::Fix> departureFix, arrivalFix;

    void showDeparturePage();
    void onDepartureEntered(const std::string &departure);

    void showArrivalPage();
    void onArrivalEntered(const std::string &arrival);

    std::shared_ptr<navdb::Route> asyncSearchForRoute();
    void showRoute();

    void showError(const std::string &msg);
    void showStatus(const std::string &msg);

    void reset();

    std::string toShortRouteDescription();
    std::string toDetailedRouteDescription();

    std::string fmsRawText;
    void selectFlightPlanFile();
    void loadRouteFromFMS(const std::filesystem::path &fmsFilename);
};

} /* namespace avitab */

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

#include "App.h"
#include "core/gui_toolkit/widgets/Container.h"
#include "core/gui_toolkit/widgets/Label.h"
#include "core/gui_toolkit/widgets/Button.h"
#include "core/gui_toolkit/widgets/Checkbox.h"
#include "core/gui_toolkit/widgets/Slider.h"
#include "core/gui_toolkit/Timer.h"
#include <array>

namespace avitab {

class HeaderApp: public App {
public:
    HeaderApp(FuncsPtr appFuncs);
private:
    static constexpr int HOR_PADDING = 10, VERT_PADDING = 10;
    std::shared_ptr<Label> clockLabel, fpsLabel;
    std::shared_ptr<Button> homeButton;
    std::shared_ptr<Button> settingsButton;;

    std::shared_ptr<Container> prefContainer;
    std::shared_ptr<Slider> brightnessSlider;
    std::shared_ptr<Checkbox> fpsCheckbox;
    std::shared_ptr<Button> pauseButton, nextButton, prevButton;
    std::shared_ptr<Label> brightLabel, mediaLabel;
    std::shared_ptr<Button> closeButton;

    std::shared_ptr<avitab::Settings> savedSettings;

    static constexpr int TIMER_PERIOD_MS = 100;
    static constexpr int TIMER_TICKS_PER_SEC = 1000 / TIMER_PERIOD_MS;
    Timer tickTimer; // managed by lvgl
    int clickTimer;
    bool clickActive = false;
    int clockUpdateAlarm = 0; // counts down, triggers GUI update at <= 0
    static constexpr unsigned int CLOCK_WRAP_SECONDS = 60 * 60 * 24;
    unsigned int elapsedTimerStartS = 2 * CLOCK_WRAP_SECONDS;
    enum clockMode {
        REAL_WORLD_TIME_LOCAL,
        SIM_TIME_ZULU,
        SIM_TIME_LOCAL,
        ELAPSED_TIMER,
        NUM_CLOCK_MODES
    };
    int curClockMode = REAL_WORLD_TIME_LOCAL;

    bool showFps = true;
    std::array<float, 30> fpsRingBuffer{};
    int fpsRingCursor = 0;

    void onScreenResize(int width, int height) override;

    void createSettingsContainer();
    void toggleSettings();
    void onBrightnessChange(int brightness);

    void onClockClick(int x, int y, bool pr, bool rel);
    bool onTick();
    unsigned int getElapsedTime();
    void updateClock();
    void updateFPS();
    void pushFPSValue(float fps);
    float getAverageFPS();
};

} /* namespace avitab */

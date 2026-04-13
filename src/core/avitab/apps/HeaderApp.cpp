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
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <ctime>
#include "HeaderApp.h"
#include "platform/Platform.h"
#include "core/Logger.h"

namespace avitab {

HeaderApp::HeaderApp(FuncsPtr appFuncs):
    App(appFuncs),
    savedSettings(appFuncs->getSettings()),
    tickTimer(std::bind(&HeaderApp::onTick, this), TIMER_PERIOD_MS)
{
    auto container = getUIContainer();
    container->setPosition(0, 0);
    container->setDimensions(container->getWidth(), 30);
    clockLabel = std::make_shared<Label>(container, "");
    clockLabel->setClickable(true);
    clockLabel->setClickHandler([this] (int x, int y, bool pr, bool rel) { onClockClick(x, y, pr, rel); });

    settingsButton = std::make_shared<Button>(container, Widget::Symbol::SETTINGS);
    settingsButton->setCallback([this] (const Button &) { toggleSettings(); });
    settingsButton->alignLeftInParent(HOR_PADDING);

    showFps = savedSettings->getGeneralSetting<bool>("show_fps");
    fpsLabel = std::make_shared<Label>(container, "-- FPS");
    fpsLabel->alignRightOf(settingsButton);
    fpsLabel->setVisible(showFps);

    homeButton = std::make_shared<Button>(container, Widget::Symbol::HOME);
    homeButton->setCallback([this] (const Button &) { api().onHomeButton(); });
    homeButton->centerInParent();

    createSettingsContainer();

    onTick();
}

void HeaderApp::onScreenResize(int width, int height) {
}

void HeaderApp::createSettingsContainer() {
    auto ui = getUIContainer();

    prefContainer = std::make_shared<Container>();
    prefContainer->setDimensions(ui->getWidth() / 2, ui->getHeight() / 2);
    prefContainer->centerInParent();
    prefContainer->setFit(Container::Fit::TIGHT, Container::Fit::TIGHT);
    prefContainer->setVisible(false);

    brightLabel = std::make_shared<Label>(prefContainer, "Brightness");
    brightLabel->alignLeftInParent(HOR_PADDING);
    brightnessSlider = std::make_shared<Slider>(prefContainer, 10, 100);
    brightnessSlider->setValue(api().getBrightness() * 100);
    brightnessSlider->setCallback([this] (int brightness) { onBrightnessChange(brightness); });
    brightnessSlider->alignRightOf(brightLabel, HOR_PADDING);

    fpsCheckbox = std::make_shared<Checkbox>(prefContainer, "Show FPS");
    fpsCheckbox->setChecked(showFps);
    fpsCheckbox->setCallback([this] (bool checked) { showFps = checked; savedSettings->setGeneralSetting<bool>("show_fps", showFps); fpsLabel->setVisible(showFps); });
    fpsCheckbox->alignRightOf(brightnessSlider);

    mediaLabel = std::make_shared<Label>(prefContainer, "Ext. Media");
    mediaLabel->alignBelow(brightLabel, VERT_PADDING);

    prevButton = std::make_shared<Button>(prefContainer, Widget::Symbol::PREV);
    prevButton->setCallback([] (const Button &) { platform::controlMediaPlayer(platform::MediaControl::MEDIA_PREV); });
    prevButton->alignBelow(brightnessSlider);

    pauseButton = std::make_shared<Button>(prefContainer, Widget::Symbol::PAUSE);
    pauseButton->setCallback([] (const Button &) { platform::controlMediaPlayer(platform::MediaControl::MEDIA_PAUSE); });
    pauseButton->alignRightOf(prevButton);

    nextButton = std::make_shared<Button>(prefContainer, Widget::Symbol::NEXT);
    nextButton->setCallback([] (const Button &) { platform::controlMediaPlayer(platform::MediaControl::MEDIA_NEXT); });
    nextButton->alignRightOf(pauseButton);

    closeButton = std::make_shared<Button>(prefContainer, "Close AviTab");
    closeButton->setCallback([this] (const Button &) { toggleSettings(); api().close(); });
    closeButton->alignBelow(mediaLabel, VERT_PADDING);
}

void HeaderApp::toggleSettings() {
    prefContainer->setVisible(!prefContainer->isVisible());
}

void HeaderApp::onBrightnessChange(int brightness) {
    api().setBrightness(brightness / 100.0f);
}

void HeaderApp::onClockClick(int x, int y, bool press, bool release) {
    if (press) {
        clickActive = true;
        clickTimer = 0;
    } else if (release) {
        // if the clock is in elapsed timer mode and the click was held for
        // more than 2 seconds then the elapsed timer is restarted.
        if ((curClockMode == ELAPSED_TIMER) && (clickTimer > (2 * TIMER_TICKS_PER_SEC))) {
            elapsedTimerStartS = api().getZuluTimeSeconds();
        } else {
            curClockMode = (curClockMode + 1) % NUM_CLOCK_MODES;
        }
        clickActive = false;
        clockUpdateAlarm = 0;
    }
}

bool HeaderApp::onTick() {
    ++clickTimer;
    if (--clockUpdateAlarm <= 0) {
        updateClock();
    }
    updateFPS();
    return true;
}

unsigned int HeaderApp::getElapsedTime() {
    return (api().getZuluTimeSeconds() + CLOCK_WRAP_SECONDS - elapsedTimerStartS) % CLOCK_WRAP_SECONDS;
}

void HeaderApp::updateClock() {
    unsigned int tMajor = 0, tMinor = 0;
    char prefix = 0, suffix = 0;

    if (curClockMode == REAL_WORLD_TIME_LOCAL) {
        // display hours and minutes, update required at/just after next minute increment
        time_t now = time(nullptr);
        tm *local = localtime(&now);
        tMajor = local->tm_hour;
        tMinor = local->tm_min;
        prefix = '{'; suffix = '}';
        clockUpdateAlarm = ((60 - local->tm_sec) + 1) * TIMER_TICKS_PER_SEC;
    } else if (curClockMode == ELAPSED_TIMER) {
        if (elapsedTimerStartS >= CLOCK_WRAP_SECONDS) { // run once to initialise
            elapsedTimerStartS = api().getZuluTimeSeconds();
        }
        if (clickActive && (clickTimer > (2 * TIMER_TICKS_PER_SEC))) {
            elapsedTimerStartS = api().getZuluTimeSeconds(); // will zero the timer after click held for 2 seconds
        }
        // display minutes and seconds, update required in 1 second
        unsigned int elapsedTime = getElapsedTime();
        tMajor = (elapsedTime / 60) % 60;
        tMinor = elapsedTime % 60;
        prefix = '+';
        clockUpdateAlarm = TIMER_TICKS_PER_SEC;
    } else { // either one of the 2 simulation time modes
        // display hours and minutes, update required at/just after next minute increment
        unsigned int simTime = (curClockMode == SIM_TIME_LOCAL) ? api().getLocalTimeSeconds() : api().getZuluTimeSeconds();
        tMajor = (simTime / (60 * 60)) % 24;
        tMinor = (simTime / 60) % 60;
        if (curClockMode == SIM_TIME_ZULU) suffix = 'z';
        clockUpdateAlarm = ((simTime % 60) + 1) * TIMER_TICKS_PER_SEC;
    }

    std::string s;
    if (prefix) s.push_back(prefix);
    s.push_back('0' + (tMajor / 10)); s.push_back('0' + (tMajor % 10));
    s.push_back(':');
    s.push_back('0' + (tMinor / 10)); s.push_back('0' + (tMinor % 10));
    if (suffix) s.push_back(suffix);

    clockLabel->setText(s);
    clockLabel->alignRightInParent(HOR_PADDING);
}

void HeaderApp::updateFPS() {
    float lastFramePeriod = api().getLastFrameTime();

    if (lastFramePeriod > 0) {
        pushFPSValue(1 / lastFramePeriod);
    }

    if (fpsRingCursor % 10 == 0) {
        float avgFps = getAverageFPS();
        if (avgFps > 0) {
            fpsLabel->setTextFormatted("%.0f FPS", avgFps);
            fpsLabel->alignRightOf(settingsButton);
        }
    }
}

void HeaderApp::pushFPSValue(float fps) {
    fpsRingBuffer[fpsRingCursor++] = fps;
    fpsRingCursor %= fpsRingBuffer.size();
}

float HeaderApp::getAverageFPS() {
    float fpsSum = 0;
    size_t count = 0;

    std::for_each(std::begin(fpsRingBuffer), std::end(fpsRingBuffer), [&fpsSum, &count] (float f) {
        if (f > 0 && f < 1000) {
            fpsSum += f;
            count++;
        }
    });

    if (count > 0) {
        return fpsSum / count;
    } else {
        return 0;
    }
}

} /* namespace avitab */

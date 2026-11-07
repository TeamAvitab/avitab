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
#include "UiDriver.h"
#include "gui/Theme.h"
#include "gui/widgets/Screen.h"

namespace avitab {

class LVGLToolkit {
public:
    LVGLToolkit(std::shared_ptr<UiDriverBase> drv, const std::string themedir, const std::string themename);
    ~LVGLToolkit();

    void doLVGLtasks();
    void doLVGLtick(int t);

    void handleKeyboard();

    void resizeScreen(int w, int h);

    void setBrightness(float b);
    float getBrightness();

    std::shared_ptr<Screen> &screen();
    std::shared_ptr<Theme> &theme();
    unsigned long getFrameCount() const { return frameCounter; }

private:
    std::shared_ptr<UiDriverBase> const driver;
    std::shared_ptr<Screen> mainScreen;
    std::shared_ptr<Theme> mainTheme;

    unsigned long frameCounter = 0;

    void initDisplay();
    void initInputDevice();

    lv_obj_t *searchActiveKeyboard(lv_obj_t *obj);
};

} /* namespace avitab */

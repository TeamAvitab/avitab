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
#include <chrono>
#include <lvgl.h>
#include "LVGLToolkit.h"
#include "widgets/Keyboard.h"
#include "platform/Platform.h"
#include "platform/CrashHandler.h"
#include "Logger.h"

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_XRGB8888))

namespace avitab {

namespace {
bool lvglIsInitialized = false;
lv_display_t *display;
lv_indev_t *inputDevice;
std::vector<uint8_t> tmpBuffer;
}

LVGLToolkit::LVGLToolkit(std::shared_ptr<UiDriverBase> drv, const std::string dataDir):
    driver(drv)
{
    driver->init(INITIAL_WIDTH, INITIAL_HEIGHT);

    if (!lvglIsInitialized) {
        lv_init();

        // LVGL does not support de-initialization so we can only do this once
        lv_log_register_print_cb([] (lv_log_level_t level, const char *msg) {
            switch (level) {
                case LV_LOG_LEVEL_INFO:
                    logger::info("GUI: %s", msg);
                    break;
                case LV_LOG_LEVEL_WARN:
                    logger::warn("GUI: %s", msg);
                    break;
                case LV_LOG_LEVEL_ERROR:
                    logger::error("GUI: %s", msg);
                    break;
                default:
                    logger::verbose("GUI: %s", msg);
                    break;
            }
        });

        lvglIsInitialized = true;
    }

    initDisplay();
    initInputDevice();

    Theme::init(dataDir + "/themes", display);

    // if keepAlive if true, the window was hidden without us noticing
    // so it's enough to re-create it without starting rendering again
    mainScreen = std::make_shared<Screen>();
    guiActive = true;
    guiThread = std::make_unique<std::thread>(&LVGLToolkit::guiLoop, this);
}

void LVGLToolkit::initDisplay() {
    static_assert(BYTES_PER_PIXEL == 4, "Invalid lvgl color type");
//    bool isUpdate = (lv_display_get_user_data(display) != nullptr);

    tmpBuffer.resize(2047 * BYTES_PER_PIXEL * 2047 * BYTES_PER_PIXEL);
    display = lv_display_create(INITIAL_WIDTH, INITIAL_HEIGHT);
    lv_display_set_user_data(display, this);
    lv_display_set_buffers(display, tmpBuffer.data(), nullptr, tmpBuffer.size(), LV_DISPLAY_RENDER_MODE_FULL);

    lv_display_set_flush_cb(display, [] (lv_display_t *display, const lv_area_t *area, uint8_t *data) {
        LVGLToolkit *us = (LVGLToolkit *)(lv_display_get_user_data(display));
        if (!us) {
            return;
        }

        int x1 = area->x1;
        int y1 = area->y1;
        int x2 = area->x2;
        int y2 = area->y2;

        us->driver->blit(x1, y1, x2, y2, reinterpret_cast<const uint32_t *>(data));
        lv_display_flush_ready(display);
    });

    driver->setResizeCallback([this] (int w, int h) {
       executeLater([w, h] {
            lv_display_set_resolution(display, std::min(w, 2047), std::min(h, 2047));
        });
    });

//    if (isUpdate) {
//        lv_disp_drv_update(lv_disp_get_default(), &lvDriver);
//    } else {
//        lv_disp_drv_register(&lvDriver);
//    }
}

void LVGLToolkit::initInputDevice() {
    bool isUpdate = (lv_indev_get_read_cb(inputDevice) != nullptr);

    if (!isUpdate) {
        inputDevice = lv_indev_create();
    }

    lv_indev_set_type(inputDevice, LV_INDEV_TYPE_POINTER);
    lv_indev_set_user_data(inputDevice, this);
    lv_indev_set_read_cb(inputDevice, [] (lv_indev_t  *inputDevice, lv_indev_data_t *data) {
        LVGLToolkit *us = (LVGLToolkit *)(lv_indev_get_user_data(inputDevice));
        if (!us) {
            return;
        }

        int x, y;
        bool pressed;
        us->driver->readPointerState(x, y, pressed);
        data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        data->point.x = x;
        data->point.y = y;
        return;
    });

//    if (isUpdate) {
//       lv_indev_drv_update(lv_indev_get_next(nullptr), &inputDriver);
//    } else {
//        lv_indev_drv_register(&inputDriver);
//    }
}

void LVGLToolkit::createNativeWindow(const std::string& title, const WindowRect &rect) {
    driver->createWindow(title, rect);
}

bool LVGLToolkit::hasNativeWindow() {
    return driver->hasWindow();
}

WindowRect LVGLToolkit::getNativeWindowRect() {
    return driver->getWindowRect();
}

void LVGLToolkit::pauseNativeWindow() {
    driver->killWindow();
}

void LVGLToolkit::createPanel(int left, int bottom, int width, int height, UiDriverBase::PanelControlMode mode) {
    driver->createPanel(left, bottom, width, height, mode);
}

void LVGLToolkit::hidePanel() {
    driver->hidePanel();
}

void LVGLToolkit::signalStop() {
    guiActive = false;
}

void LVGLToolkit::destroyNativeWindow() {
    if (guiThread) {
        guiActive = false;
        guiThread->join();
        guiThread.reset();
        mainScreen.reset();
        driver->setWantKeyInput(false);
        driver->hidePanel();
        driver->killWindow();
    }
}

std::shared_ptr<Screen> &LVGLToolkit::screen() {
    return mainScreen;
}

void LVGLToolkit::setMouseWheelCallback(MouseWheelCallback cb) {
    onMouseWheel = cb;
}

void LVGLToolkit::setBrightness(float b) {
    driver->setBrightness(b);
}

float LVGLToolkit::getBrightness() {
    return driver->getBrightness();
}

void LVGLToolkit::guiLoop() {
    using namespace std::chrono_literals;
    crash::ThreadCookie crashCookie;

    logger::verbose("LVGL thread has id %d", std::this_thread::get_id());

    while (guiActive) {
        // chrono clocks run at 64Hz precision in mingw, use something custom
        auto startAt = platform::measureTime();

        try {
            // first run the actual GUI tasks, i.e. let LVGL do its animations etc.
            lv_timer_handler();

            // then run our own tasks
            // grab the mutex briefly to extract the tasks into a local list
            // this avoids potential race and deadlock conditions.
            std::vector<GUITask> tasks;
            {
                std::lock_guard<std::recursive_mutex> lock(guiMutex);
                tasks = pendingTasks;
                pendingTasks.clear();
            }

            for (GUITask &task: tasks) {
                task();
            }

            // NB mouse button handling is done in the input driver callback
            handleMouseWheel(driver->getWheelClicks());
            handleKeyboard();
        } catch (const std::exception &e) {
            logger::error("Exception in GUI: %s", e.what());
        }

        std::this_thread::sleep_for(1ms);
        auto elapsedMillis = platform::getElapsedMillis(startAt);

        lv_tick_inc(std::max(elapsedMillis, 1));
    }

    logger::verbose("LVGL thread destroyed");
}

void LVGLToolkit::handleMouseWheel(int clicks) {
    if (clicks && onMouseWheel) {
        int x, y;
        bool ignored;
        driver->readPointerState(x, y, ignored);
        onMouseWheel(clicks, x, y);
    }
}

void LVGLToolkit::handleKeyboard() {
    // check if we want keys
    auto keyboard = searchActiveKeyboard(lv_screen_active());

    // then process keys
    uint32_t c = 0;
    while ((c = driver->popKeyPress()) != 0) {
        if (keyboard) {
            auto ta = lv_keyboard_get_textarea(keyboard);
            auto keyb = (Keyboard *) lv_obj_get_user_data(keyboard);
            if (!ta) {
                continue;
            }

            if (c == '\b') {
                lv_textarea_delete_char(ta);
            } else if (c == '\n') {
                if (keyb && keyb->hasOkAction()) {
                    lv_obj_send_event(keyboard, LV_EVENT_READY, nullptr);
                } else {
                    lv_textarea_add_char(ta, '\n');
                }
            } else {
                lv_textarea_add_char(ta, lv_text_encoded_conv_wc(c));
            }
        }
    }

    driver->setWantKeyInput(keyboard != nullptr);
}

lv_obj_t *LVGLToolkit::searchActiveKeyboard(lv_obj_t* obj) {
    if (!obj || lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) {
        return nullptr;
    }

    lv_obj_t *screen = lv_screen_active();
    if (screen) {
        if (!lv_area_is_on(&screen->coords, &obj->coords)) {
            return nullptr;
        }
    }

    uint32_t i;
    for(i = 0; i < lv_obj_get_child_cnt(obj); i++) {
    lv_obj_t * curChild = lv_obj_get_child(obj, i);
        lv_obj_t *keyb = searchActiveKeyboard(curChild);
        if (keyb) {
            return keyb;
        }

        if (lv_obj_check_type(curChild, &lv_keyboard_class)) {
            if (lv_area_is_in(&curChild->coords, &screen->coords, 0)) {
                return curChild;
            }
        }
    }
    return nullptr;
}

void LVGLToolkit::executeLater(GUITask func) {
    std::lock_guard<std::recursive_mutex> lock(guiMutex);
    if (guiActive) {
        pendingTasks.push_back(func);
    }
}

LVGLToolkit::~LVGLToolkit() {
    logger::verbose("~LVGLToolkit");
//    inputDriver.user_data = nullptr;
//    lvDriver.user_data = nullptr;
    lv_indev_set_user_data(inputDevice, nullptr);
    lv_display_set_user_data(display, nullptr);
    destroyNativeWindow();
}

} /* namespace avitab */

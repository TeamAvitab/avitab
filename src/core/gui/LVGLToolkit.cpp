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
#include "Logger.h"

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_XRGB8888))

namespace avitab {

namespace {
lv_display_t *display;
lv_indev_t *inputDevice;
std::vector<uint8_t> tmpBuffer;
}

LVGLToolkit::LVGLToolkit(std::shared_ptr<UiDriverBase> drv, const std::string themedir, const std::string themename):
    driver(drv)
{
    static bool lvglIsInitialized = false;
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

    mainTheme = std::make_shared<Theme>(themedir, themename, display);

    mainScreen = std::make_shared<Screen>();
}

LVGLToolkit::~LVGLToolkit() {
    logger::verbose("~LVGLToolkit");
    driver->setWantKeyInput(false);
    mainScreen.reset();
    mainTheme.reset();
    lv_indev_set_user_data(inputDevice, nullptr);
    lv_display_set_user_data(display, nullptr);
}

void LVGLToolkit::doLVGLtasks() {
    lv_timer_handler();
}

void LVGLToolkit::doLVGLtick(int t) {
    lv_tick_inc(std::max(t, 1));
}

void LVGLToolkit::initDisplay() {
    static_assert(BYTES_PER_PIXEL == 4, "Invalid lvgl color type");
//    bool isUpdate = (lv_display_get_user_data(display) != nullptr);

    tmpBuffer.resize(2047 * BYTES_PER_PIXEL * 2047 * BYTES_PER_PIXEL);
    display = lv_display_create(UiDriverBase::INITIAL_WIDTH, UiDriverBase::INITIAL_HEIGHT);
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
        ++us->frameCounter;
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

void LVGLToolkit::resizeScreen(int w, int h) {
    //FIXME 2047
    lv_display_set_resolution(display, std::min(w, 2047), std::min(h, 2047));
}

std::shared_ptr<Screen> &LVGLToolkit::screen() {
    return mainScreen;
}

std::shared_ptr<Theme> &LVGLToolkit::theme() {
    return mainTheme;
}

void LVGLToolkit::setBrightness(float b) {
    driver->setBrightness(b);
}

float LVGLToolkit::getBrightness() {
    return driver->getBrightness();
}

void LVGLToolkit::handleKeyboard() {
    // check if the current widget hierarchy includes an active keyboard
    auto keyboard = searchActiveKeyboard(lv_screen_active());
    driver->setWantKeyInput(keyboard != nullptr);

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

} /* namespace avitab */

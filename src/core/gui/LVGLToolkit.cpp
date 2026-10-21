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

namespace avitab {

namespace {
lv_disp_buf_t lvDispBuf;
lv_disp_drv_t lvDriver;
lv_indev_drv_t inputDriver;
std::vector<uint32_t> tmpBuffer;
}

LVGLToolkit::LVGLToolkit(std::shared_ptr<UiDriverBase> drv):
    driver(drv)
{
    static bool lvglIsInitialized = false;
    if (!lvglIsInitialized) {
        // LVGL does not support de-initialization so we can only do this once
        lv_log_register_print_cb([] (lv_log_level_t level, const char *file, uint32_t line, const char *msg) {
            switch (level) {
                case LV_LOG_LEVEL_WARN:
                    logger::warn("GUI: %s:%d %s", file, line, msg);
                    break;
                case LV_LOG_LEVEL_ERROR:
                    logger::error("GUI: %s:%d %s", file, line, msg);
                    break;
                default:
                    logger::verbose("GUI: %s:%d %s", file, line, msg);
                    break;
            }
        });

        lv_init();

        lv_theme_t *theme = lv_theme_night_init(210, LV_FONT_DEFAULT);
        lv_theme_set_current(theme);
        lvglIsInitialized = true;
    }

    initDisplayDriver();
    initInputDriver();

    mainScreen = std::make_shared<Screen>();
}

LVGLToolkit::~LVGLToolkit() {
    logger::verbose("~LVGLToolkit");
    driver->setWantKeyInput(false);
    mainScreen.reset();
    inputDriver.user_data = nullptr;
    lvDriver.user_data = nullptr;
}

void LVGLToolkit::doLVGLtasks() {
    lv_task_handler();
}

void LVGLToolkit::doLVGLtick(int t) {
    lv_tick_inc(std::max(t, 1));
}

void LVGLToolkit::initDisplayDriver() {
    static_assert(sizeof(lv_color_t) == sizeof(uint32_t), "Invalid lvgl color type");
    bool isUpdate = (lvDriver.buffer != nullptr);

    tmpBuffer.resize(LV_HOR_RES_MAX * LV_VER_RES_MAX);
    lv_disp_buf_init(&lvDispBuf, tmpBuffer.data(), nullptr, tmpBuffer.size());

    lv_disp_drv_init(&lvDriver);

    lvDriver.user_data = this;
    lvDriver.hor_res = UiDriverBase::INITIAL_WIDTH;
    lvDriver.ver_res = UiDriverBase::INITIAL_HEIGHT;
    lvDriver.buffer = &lvDispBuf;

    lvDriver.flush_cb = [] (lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *data) {
        LVGLToolkit *us = (LVGLToolkit *) drv->user_data;
        if (!us) {
            return;
        }

        int x1 = area->x1;
        int y1 = area->y1;
        int x2 = area->x2;
        int y2 = area->y2;

        us->driver->blit(x1, y1, x2, y2, reinterpret_cast<const uint32_t *>(data));
        lv_disp_flush_ready(drv);
        ++us->frameCounter;
    };

    if (isUpdate) {
        lv_disp_drv_update(lv_disp_get_default(), &lvDriver);
    } else {
        lv_disp_drv_register(&lvDriver);
    }
}

void LVGLToolkit::initInputDriver() {
    bool isUpdate = (inputDriver.read_cb != nullptr);

    if (!isUpdate) {
        lv_indev_drv_init(&inputDriver);
    }

    inputDriver.type = LV_INDEV_TYPE_POINTER;
    inputDriver.user_data = this;
    inputDriver.read_cb = [] (lv_indev_drv_t  *drv, lv_indev_data_t *data) -> bool {
        LVGLToolkit *us = (LVGLToolkit *) drv->user_data;
        if (!us) {
            return false;
        }

        int x, y;
        bool pressed;
        us->driver->readPointerState(x, y, pressed);
        data->state = pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
        data->point.x = x;
        data->point.y = y;
        return false;
    };

    if (isUpdate) {
        lv_indev_drv_update(lv_indev_get_next(nullptr), &inputDriver);
    } else {
        lv_indev_drv_register(&inputDriver);
    }
}

void LVGLToolkit::resizeScreen(int w, int h) {
    lvDriver.hor_res = std::min(w, LV_HOR_RES_MAX);
    lvDriver.ver_res = std::min(h, LV_VER_RES_MAX);
    lv_disp_drv_update(lv_disp_get_default(), &lvDriver);
}

std::shared_ptr<Screen> &LVGLToolkit::screen() {
    return mainScreen;
}

void LVGLToolkit::setBrightness(float b) {
    driver->setBrightness(b);
}

float LVGLToolkit::getBrightness() {
    return driver->getBrightness();
}

void LVGLToolkit::handleKeyboard() {
    // check if the current widget hierarchy includes an active keyboard
    auto keyboard = searchActiveKeyboard(lv_scr_act());
    driver->setWantKeyInput(keyboard != nullptr);

    // then process keys
    uint32_t c = 0;
    while ((c = driver->popKeyPress()) != 0) {
        if (keyboard) {
            auto ta = lv_kb_get_ta(keyboard);
            auto keyb = (Keyboard *) lv_obj_get_user_data(keyboard);
            if (!ta) {
                continue;
            }

            if (c == '\b') {
                lv_ta_del_char(ta);
            } else if (c == '\n') {
                if (keyb && keyb->hasOkAction()) {
                    lv_obj_get_event_cb(keyboard)(keyboard, LV_EVENT_APPLY);
                } else {
                    lv_ta_add_char(ta, '\n');
                }
            } else {
                lv_ta_add_char(ta, lv_txt_encoded_conv_wc(c));
            }
        }
    }
}

lv_obj_t *LVGLToolkit::searchActiveKeyboard(lv_obj_t* obj) {
    if (!obj || lv_obj_get_hidden(obj)) {
        return nullptr;
    }

    lv_obj_t *screen = lv_scr_act();
    if (screen) {
        if (!lv_area_is_on(&screen->coords, &obj->coords)) {
            return nullptr;
        }
    }

    lv_obj_t *curChild = nullptr;
    while ((curChild = lv_obj_get_child(obj, curChild)) != nullptr) {
        lv_obj_t *keyb = searchActiveKeyboard(curChild);
        if (keyb) {
            return keyb;
        }

        lv_obj_type_t type{};
        lv_obj_get_type(curChild, &type);
        if (strcmp(type.type[0], "lv_kb") == 0) {
            if (lv_area_is_in(&curChild->coords, &screen->coords)) {
                return curChild;
            }
        }
    }
    return nullptr;
}

} /* namespace avitab */

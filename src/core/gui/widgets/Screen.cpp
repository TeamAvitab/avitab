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
#include "Screen.h"

namespace avitab {

Screen::Screen():
    Widget(nullptr)
{
    lv_obj_t *obj = lv_screen_active();

    lv_obj_set_user_data(obj, this);

    lv_obj_add_event_cb(obj, [] (lv_event_t *e) {
        lv_obj_t *o = lv_event_get_target_obj(e);
        Screen *us = (Screen *) lv_obj_get_user_data(o);

        if (us->onResize) {
            us->onResize();
        }
    }, LV_EVENT_SIZE_CHANGED, nullptr);

    // we only use one screen, thus theme is set only once
    lv_theme_t *lvTheme = lv_theme_get_from_obj(obj);
    setTheme(reinterpret_cast<Theme *>(lvTheme->user_data)->shared_from_this());

/*
    originalSignalCB = lv_obj_get_signal_cb(obj);
    lv_obj_set_signal_cb(obj, [] (lv_obj_t *obj, lv_signal_t sig, void *param) -> lv_res_t  {
        Screen *us = (Screen *) lv_obj_get_user_data(obj);

        if (sig == LV_SIGNAL_CORD_CHG) {
            if (us->onResize) {
                us->onResize();
            }
        }

        return us->originalSignalCB(obj, sig, param);
    });
*/
    setObj(obj);
    setManaged();
}

Screen::~Screen() {
    // FIXME
    //lv_obj_set_signal_cb(obj(), originalSignalCB);
}

void Screen::setOnResize(Screen::ResizeCB cb) {
    onResize = cb;
}

} /* namespace avitab */

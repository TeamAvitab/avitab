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
#include <src/misc/lv_style.h>
#include "Button.h"
#include "Logger.h"

namespace avitab {

Button::Button(WidgetPtr parent, const std::string& text):
    Widget(parent)
{
    lv_obj_t *button = lv_button_create(parentObj());
    // FIXME
    //lv_cont_set_fit(button, LV_FIT_TIGHT);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text.c_str());
    lv_obj_center(label);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);

    setObj(button);
}

Button::Button(WidgetPtr parent, const std::filesystem::path& icon, const std::string& caption, int width):
    Widget(parent)
{
    lv_obj_t *button = lv_button_create(parentObj());

    if (width > 0) {
        lv_obj_set_width(button, width);
    }

    lv_obj_t *ico = lv_image_create(button);
    lv_obj_clear_flag(ico, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ico, icon.c_str());
    lv_obj_align(ico, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *label = lv_label_create(button);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_label_set_text(label, caption.c_str());
    lv_obj_align_to(label, ico, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    setObj(button);
}

Button::Button(WidgetPtr parent, img::Image &&icon, const std::string& caption, int width):
    Widget(parent)
{
    iconData = std::move(icon);
    iconImage = toLVImage(iconData.getPixels(), iconData.getWidth(), iconData.getHeight());

    lv_obj_t *button = lv_button_create(parentObj());
    lv_obj_set_height(button, LV_SIZE_CONTENT);

    if (width >=0) {
        lv_obj_set_width(button, width);
    }

    lv_obj_t *ico = lv_image_create(button);
    lv_obj_clear_flag(ico, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ico, &iconImage);
    lv_obj_align(ico, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *label = lv_label_create(button);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_label_set_text(label, caption.c_str());
    lv_obj_align_to(label, ico, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

 /* FIXME
    if (width >= 0) {
        lv_cont_set_fit2(button, LV_FIT_NONE, LV_FIT_TIGHT);
        lv_obj_set_width(button, width);
    } else {
    lv_cont_set_fit(button, LV_FIT_TIGHT);
    }
*/
    setObj(button);
}

Button::Button(WidgetPtr parent, Symbol smb):
    Widget(parent)
{
    lv_obj_t *button = lv_button_create(parentObj());
    // FIXME
    //lv_cont_set_fit(button, LV_FIT_TIGHT);

    lv_obj_t *ico = lv_image_create(button);
    lv_image_set_src(ico, symbolToLVSymbol(smb));
    lv_obj_center(ico);
    lv_obj_clear_flag(ico, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(button, LV_OPA_TRANSP, LV_PART_MAIN);

    setObj(button);
}

Button::Button(WidgetPtr parent, lv_obj_t* obj):
    Widget(parent)
{
    setManagedObj(obj);
}

void Button::setFit(bool hor, bool vert) {
// FIXME
//    lv_cont_set_fit2(obj(), hor, vert);
}

void Button::setCallback(ButtonCallback cb) {
    callbackFunc = cb;
    lv_obj_set_user_data(obj(), this);

    lv_obj_add_event_cb(obj(), [] (lv_event_t *e) {
        lv_obj_t *o = lv_event_get_target_obj(e);
        Button *us = reinterpret_cast<Button *>(lv_obj_get_user_data(o));
        if (us->callbackFunc) {
            us->callbackFunc(*us);
        }
    }, LV_EVENT_CLICKED, nullptr);
}

void Button::setToggleable(bool toggleable) {
    if (toggleable) {
        lv_obj_add_flag(obj(), LV_OBJ_FLAG_CHECKABLE);
    } else {
        lv_obj_clear_flag(obj(), LV_OBJ_FLAG_CHECKABLE);
    }
}

void Button::setToggleState(bool toggled) {
    if (toggled) {
        lv_obj_add_state(obj(), LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(obj(), LV_STATE_CHECKED);
    }
}

} /* namespace avitab */

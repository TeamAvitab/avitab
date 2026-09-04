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

#include <stdexcept>
#include "Widget.h"
#include "gui/LVGLToolkit.h"

namespace avitab {

Widget::Widget(WidgetPtr parent):
    parent(parent)
{
}

void Widget::setObj(lv_obj_t* obj) {
    if (obj == nullptr) {
        throw std::runtime_error("NULL object passed to setObj");
    }
    lvObj = obj;
}

lv_obj_t* Widget::obj() {
    return lvObj;
}

lv_obj_t* Widget::parentObj() {
    if (parent) {
        return parent->obj();
    } else {
        return nullptr;
    }
}

void Widget::setParent(WidgetPtr newParent) {
    parent = newParent;
    lv_obj_set_parent(obj(), newParent->obj());
}

void Widget::setPosition(int x, int y) {
    lv_obj_set_pos(obj(), x, y);
}

void Widget::showInFront() {
    lv_obj_move_foreground(obj());
}

void Widget::showInBack() {
    lv_obj_move_background(obj());
}

void Widget::setClickable(bool click) {
    if (click) {
        lv_obj_add_flag(obj(), LV_OBJ_FLAG_CLICKABLE);
    } else {
        lv_obj_clear_flag(obj(), LV_OBJ_FLAG_CLICKABLE);
    }
}

void Widget::setDimensions(int width, int height) {
    lv_obj_set_size(obj(), width, height);
}

void Widget::setWidth(int width) {
    lv_obj_set_width(lvObj, width);
}

void Widget::setHeight(int height) {
    lv_obj_set_height(lvObj, height);
}

void Widget::setDimensionsPct(int width, int height) {
    lv_obj_set_size(obj(), lv_pct(width), lv_pct(height));
}

void Widget::setWidthPct(int width) {
    lv_obj_set_width(lvObj, lv_pct(width));
}

void Widget::setHeightPct(int height) {
    lv_obj_set_height(lvObj, lv_pct(height));
}

void Widget::setSizeByContent() {
    lv_obj_set_size(obj(), LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}

void Widget::centerInParent() {
    lv_obj_align(lvObj, LV_ALIGN_CENTER, 0, 0);
}

void Widget::alignLeftInParent(int padLeft) {
    lv_obj_align(lvObj, LV_ALIGN_LEFT_MID, padLeft, 0);
}

void Widget::alignRightInParent(int padRight) {
    lv_obj_align(lvObj, LV_ALIGN_RIGHT_MID, -padRight, 0);
}

void Widget::alignTopRightInParent(int padRight, int padTop) {
    lv_obj_align(lvObj, LV_ALIGN_TOP_RIGHT, -padRight, padTop);
}

void Widget::alignInBottomLeft() {
    lv_obj_align(lvObj, LV_ALIGN_BOTTOM_LEFT, 0, 0);
}

void Widget::alignInBottomCenter() {
    lv_obj_align(lvObj, LV_ALIGN_BOTTOM_MID, 0, 0);
}

void Widget::alignInBottomRight() {
    lv_obj_align(lvObj, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
}

void Widget::alignInTopLeft() {
    lv_obj_align(lvObj, LV_ALIGN_TOP_LEFT, 0, 0);
}

void Widget::alignInTopRight(int xPad) {
    lv_obj_align(lvObj, LV_ALIGN_TOP_RIGHT, -xPad, 0);
}

void Widget::alignLeftOf(WidgetPtr base, int xPad) {
    lv_obj_align_to(lvObj, base->obj(), LV_ALIGN_OUT_LEFT_MID, 0, 0);
}

void Widget::alignRightOf(WidgetPtr base, int xPad) {
    lv_obj_align_to(lvObj, base->obj(), LV_ALIGN_OUT_RIGHT_MID, xPad, 0);
}

void Widget::alignBelow(WidgetPtr base, int yPad) {
    lv_obj_align_to(lvObj, base->obj(), LV_ALIGN_OUT_BOTTOM_LEFT, 0, yPad);
}

int Widget::getWidth() {
    lv_obj_update_layout(obj());
    return lv_obj_get_width(obj());
}

int Widget::getHeight() {
    lv_obj_update_layout(obj());
    return lv_obj_get_height(obj());
}

void Widget::setVisible(bool visible) {
    if (visible) {
        lv_obj_clear_flag(obj(), LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(obj(), LV_OBJ_FLAG_HIDDEN);
    }
}

bool Widget::isVisible() {
    return !lv_obj_has_flag(obj(), LV_OBJ_FLAG_HIDDEN);
}

void Widget::setShowScrollbar(bool show) {
    lv_obj_set_scrollbar_mode(obj(), show ? LV_SCROLLBAR_MODE_AUTO : LV_SCROLLBAR_MODE_OFF);
}

void Widget::setScrollable(bool scrollable) {
    if (scrollable) {
        lv_obj_add_flag(obj(), LV_OBJ_FLAG_SCROLLABLE);
        //lv_obj_set_scrollbar_mode();
    } else {
        lv_obj_clear_flag(obj(), LV_OBJ_FLAG_SCROLLABLE);
    }
}

bool Widget::isScrollable() {
    return lv_obj_has_flag(obj(), LV_OBJ_FLAG_SCROLLABLE);
}

void Widget::setBGColor (int color) {
    lv_obj_set_style_bg_color(obj(), lv_color_hex(color), LV_PART_MAIN);
}

void Widget::setBGTransparent () {
    lv_obj_set_style_bg_opa(obj(), LV_OPA_TRANSP, 0);
}

void Widget::setPadding(int left, int right, int top, int bottom) {
    lv_obj_set_style_pad_left(lvObj, left, 0);
    lv_obj_set_style_pad_right(lvObj, right, 0);
    lv_obj_set_style_pad_top(lvObj, top, 0);
    lv_obj_set_style_pad_bottom(lvObj, bottom, 0);
}

void Widget::setPaddingHor(int padHor) {
    lv_obj_set_style_pad_hor(lvObj, padHor, 0);
}

void Widget::setPaddingVer(int padVer) {
    lv_obj_set_style_pad_ver(lvObj, padVer, 0);
}

void Widget::setLocalStyle(const std::string style) {
    LVGLToolkit::theme().setLocalStyle(lvObj, style);
}

lv_image_dsc_t Widget::toLVImage(const uint32_t* pix, int width, int height) {
    lv_image_dsc_t res;

    res.header.magic = LV_IMAGE_HEADER_MAGIC;
    res.header.cf = LV_COLOR_FORMAT_ARGB8888;
    res.header.flags = 0;
    res.header.w = width;
    res.header.h = height;
    res.header.stride = 0;

    res.data_size = width * height * sizeof(uint32_t);
    res.data = reinterpret_cast<const uint8_t *>(pix);

    return res;
}

void Widget::invalidate() {
    lv_obj_invalidate(obj());
}

int Widget::getX() {
    return lv_obj_get_x(obj());
}

int Widget::getY() {
    return lv_obj_get_y(obj());
}

/* FIXME N/A in v8 but not used atm
void Widget::enablePanning() {
    lv_obj_set_drag(obj(), true);
}
*/

void Widget::setClickHandler(ClickHandler handler) {
    onClick = handler;
    lv_obj_set_user_data(obj(), this);

    lv_obj_add_event_cb(obj(), [] (lv_event_t *e) {
        lv_obj_t *o = lv_event_get_target_obj(e);
        lv_event_code_t ev = lv_event_get_code(e);
        Widget *us = reinterpret_cast<Widget *>(lv_obj_get_user_data(o));
        if (ev == LV_EVENT_PRESSED || ev == LV_EVENT_PRESSING || ev == LV_EVENT_RELEASED || ev == LV_EVENT_PRESS_LOST) {
            lv_point_t point;
            lv_indev_t *dev = lv_event_get_indev(e);
            lv_indev_get_point(dev, &point);
            if (us->onClick) {
                bool start = (ev == LV_EVENT_PRESSED);
                bool end = (ev == LV_EVENT_RELEASED) || (ev == LV_EVENT_PRESS_LOST);
                lv_area_t coords;
                lv_obj_get_coords(o, &coords);
                us->onClick(point.x - coords.x1, point.y - coords.y1, start, end);
            }
        }
    }, LV_EVENT_ALL, nullptr);
/*
    lv_obj_set_signal_cb(obj(), [] (_lv_obj_t *o, lv_signal_t sign, void *param) -> lv_res_t {
        Widget *us = reinterpret_cast<Widget *>(lv_obj_get_user_data(o));
        if (sign == LV_SIGNAL_PRESSED || sign == LV_SIGNAL_PRESSING || sign == LV_SIGNAL_RELEASED || sign == LV_SIGNAL_PRESS_LOST) {
            lv_point_t point;
            lv_indev_t *dev = reinterpret_cast<lv_indev_t *>(param);
            lv_indev_get_point(dev, &point);
            if (us->onClick) {
                bool start = (sign == LV_SIGNAL_PRESSED);
                bool end = (sign == LV_SIGNAL_RELEASED) || (sign == LV_SIGNAL_PRESS_LOST);
                us->onClick(point.x - o->coords.x1, point.y - o->coords.y1, start, end);
            }
            return LV_RES_OK;
        } else {
            return us->origSigFunc(o, sign, param);
        }
    });
*/
}

const void* Widget::symbolToLVSymbol(Symbol symbol) {
    const char *res = nullptr;
    switch (symbol) {
    case Symbol::NONE:      res = nullptr; break;
    case Symbol::CLOSE:     res = LV_SYMBOL_CLOSE; break;
    case Symbol::SETTINGS:  res = LV_SYMBOL_SETTINGS; break;
    case Symbol::LIST:      res = LV_SYMBOL_LIST; break;
    case Symbol::LEFT:      res = LV_SYMBOL_LEFT; break;
    case Symbol::RIGHT:     res = LV_SYMBOL_RIGHT; break;
    case Symbol::UP:        res = LV_SYMBOL_UP; break;
    case Symbol::DOWN:      res = LV_SYMBOL_DOWN; break;
    case Symbol::ROTATE:    res = LV_SYMBOL_LOOP; break;
    case Symbol::REFRESH:   res = LV_SYMBOL_REFRESH; break;
    case Symbol::PREV:      res = LV_SYMBOL_PREV; break;
    case Symbol::NEXT:      res = LV_SYMBOL_NEXT; break;
    case Symbol::PAUSE:     res = LV_SYMBOL_PAUSE; break;
    case Symbol::PLUS:      res = LV_SYMBOL_PLUS; break;
    case Symbol::MINUS:     res = LV_SYMBOL_MINUS; break;
    case Symbol::FILE:      res = LV_SYMBOL_FILE; break;
    case Symbol::DIRECTORY: res = LV_SYMBOL_DIRECTORY; break;
    case Symbol::HOME:      res = LV_SYMBOL_HOME; break;
    case Symbol::COPY:      res = LV_SYMBOL_COPY; break;
    case Symbol::GPS:       res = LV_SYMBOL_GPS; break;
    case Symbol::EDIT:      res = LV_SYMBOL_EDIT; break;
    case Symbol::KEYBOARD:  res = LV_SYMBOL_KEYBOARD; break;
    case Symbol::IMAGE:     res = LV_SYMBOL_IMAGE; break;
    default:                res = nullptr;
    }

    if (res) {
        lv_image_cache_drop(res);
    }
    return res;
}

void Widget::setManagedObj(lv_obj_t* obj) {
    managed = true;
    lvObj = obj;
}

void Widget::setManaged() {
    managed = true;
}

Widget::~Widget() {
    if (!managed) {
        lv_obj_delete(obj());
    }
}

} // namespace avitab

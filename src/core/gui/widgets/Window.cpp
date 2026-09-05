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
#include "Window.h"
#include <stdexcept>

namespace avitab {

Window::Window(WidgetPtr parent, const std::string& title, const int height):
    Widget(parent)
{
    lv_obj_t *win = lv_win_create(parentObj());
    caption = lv_win_add_title(win, title.c_str());
    lv_obj_set_height(win, height);
    lv_obj_set_user_data(win, this);

    setObj(win);
    contentArea = std::make_shared<Container>(nullptr, lv_win_get_content(win));
    // FIXME make contentArea aware the window 'owns' it. do we need to?
    //contentArea->setParent(*this);
}

void Window::setCaption(const std::string& title) {
    lv_label_set_text(caption, title.c_str());
}

std::shared_ptr<Container> Window::getContent() {
    return contentArea;
}

void Window::hideScrollbars() {
    lv_obj_clear_flag(obj(), LV_OBJ_FLAG_SCROLLABLE);
}

void Window::getHeaderArea(int &x1, int &y1, int &x2, int &y2) {
    auto header = lv_win_get_header(obj());
    lv_area_t area;
    lv_obj_get_coords(header, &area);
    x1 = area.x1;
    y1 = area.y1;
    x2 = area.x2;
    y2 = area.y1 + lv_obj_get_height(header);
}

int Window::getContentWidth() {
    lv_obj_update_layout(obj());
    return lv_obj_get_width(contentArea->obj());
}

int Window::getContentHeight() {
    lv_obj_update_layout(obj());
    return lv_obj_get_height(contentArea->obj());
}

void Window::setOnClose(WindowCallback cb) {
    addSymbol(Symbol::CLOSE, cb);
}

std::shared_ptr<Button> Window::addSymbol(Symbol smb, WindowCallback cb) {
    callbacks[smb] = cb;

    const void *lvSymbol = symbolToLVSymbol(smb);
    if (!lvSymbol) {
        throw std::runtime_error("Invalid symbol passed to window");
    }
    // FIXME button width 20
    lv_obj_t *btn = lv_win_add_button(obj(), lvSymbol, 20);
    lv_obj_add_event_cb(btn, [] (lv_event_t *e) {
        lv_obj_t *btn = lv_event_get_target_obj(e);
        lv_obj_t *winObj = lv_obj_get_parent(lv_obj_get_parent(btn));
        Window *winCls = reinterpret_cast<Window *>(lv_obj_get_user_data(winObj));
        int smbInt = reinterpret_cast<intptr_t>(lv_obj_get_user_data(btn));

        if (winCls) {
            winCls->callbacks[static_cast<Symbol>(smbInt)]();
        }
    }, LV_EVENT_CLICKED, nullptr);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_user_data(btn, reinterpret_cast<void *>(smb));
/*
    lv_obj_set_event_cb(btn, [] (lv_obj_t *btn, lv_event_t ev) {
        if (ev == LV_EVENT_CLICKED) {
            lv_obj_t *winObj = lv_win_get_from_btn(btn);
            Window *winCls = reinterpret_cast<Window *>(lv_obj_get_user_data(winObj));
            int smbInt = reinterpret_cast<intptr_t>(lv_obj_get_user_data(btn));

            if (winCls) {
                winCls->callbacks[static_cast<Symbol>(smbInt)]();
            }
        }
    });
    lv_obj_set_user_data(btn, reinterpret_cast<void *>(smb));
*/
    return std::make_shared<Button>(nullptr, btn);
}

} /* namespace avitab */

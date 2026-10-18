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
#include <string>
#include <stdexcept>
#include "TabGroup.h"
#include "Logger.h"

namespace avitab {

TabGroup::TabGroup(WidgetPtr parent):
    Widget(parent)
{
    // FIXME tab_height/tab_width
    lv_obj_t *tabs = lv_tabview_create(parentObj());
    lv_tabview_set_tab_bar_position(tabs, LV_DIR_TOP);
    lv_tabview_set_tab_bar_size(tabs, 20);

    setObj(tabs);
}

void TabGroup::setCallback(TabChangeCallback cb) {
    callbackFunc = cb;
    lv_obj_set_user_data(obj(), this);

    lv_obj_add_event_cb(obj(), [] (lv_event_t *e) {
        lv_obj_t *o = lv_event_get_target_obj(e);
        TabGroup *me = reinterpret_cast<TabGroup *>(lv_obj_get_user_data(o));
        me->callbackFunc();
    }, LV_EVENT_VALUE_CHANGED, nullptr);
}

std::shared_ptr<Page> TabGroup::addTab(WidgetPtr tabs, const std::string &title) {
    lv_obj_t *page = lv_tabview_add_tab(obj(), title.c_str());

    auto pageWidget = std::make_shared<Page>(tabs, page);
    return pageWidget;
}

void TabGroup::showTab(WidgetPtr tab) {
    setActiveTab(getTabIndex(tab));
}

void TabGroup::delTab(WidgetPtr tab) {
    removeTab(getTabIndex(tab));
}

size_t TabGroup::getTabIndex(WidgetPtr tab) {
    lv_obj_t *content = lv_tabview_get_content(obj());
    size_t i;
    for (i = 0; i < lv_obj_get_child_cnt(content); i++) {
        lv_obj_t *cur = lv_obj_get_child(content, i);
        if (cur == tab->obj()) {
            return i;
        }
    }
    throw std::runtime_error("Tab not part of tab group");
}

void TabGroup::setActiveTab(size_t i) {
    lv_tabview_set_active(obj(), i, LV_ANIM_ON);
}

size_t TabGroup::getActiveTab() {
    return lv_tabview_get_tab_active(obj());
}

void TabGroup::setSliding(bool sliding) {
    if (sliding) {
        lv_obj_add_flag(lv_tabview_get_content(obj()), LV_OBJ_FLAG_SCROLLABLE);
    }
    else {
        lv_obj_clear_flag(lv_tabview_get_content(obj()), LV_OBJ_FLAG_SCROLLABLE);
    }
}

size_t TabGroup::getTabCount() {
    lv_obj_t *content = lv_tabview_get_content(obj());
    size_t i;
    return lv_obj_get_child_cnt(content);
}

void TabGroup::removeTab(size_t i) {
    lv_tabview_delete_tab(obj(), i);
}

void TabGroup::clear() {
    lv_obj_clean(obj());
}

} /* namespace avitab */

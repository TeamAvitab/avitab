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
#include "DropDownList.h"

namespace avitab {

DropDownList::DropDownList(WidgetPtr parent, const std::vector<std::string>& choices):
    Widget(parent)
{
    lv_obj_t *obj = lv_dropdown_create(parentObj());

    std::string choiceStr;

    items = choices.size();
    for (auto &choice: choices) {
        choiceStr += choice + "\n";
    }

    if (!choiceStr.empty()) {
        choiceStr.pop_back();
    }

    lv_dropdown_set_options(obj, choiceStr.c_str());
    lv_obj_set_user_data(obj, this);

    setObj(obj);
}

int DropDownList::getSelectedIndex() {
    return lv_dropdown_get_selected(obj());
}

void DropDownList::setSymbol(Symbol smb) {
    const void *lvSymbol = symbolToLVSymbol(smb);
    if (!lvSymbol) {
        throw std::runtime_error("Invalid symbol passed to dropdown");
    }

    switch (smb) {
        case Symbol::LEFT:
            lv_dropdown_set_symbol(obj(), lvSymbol);
            lv_dropdown_set_dir(obj(), LV_DIR_LEFT);
            break;
        case Symbol::RIGHT:
            lv_dropdown_set_symbol(obj(), lvSymbol);
            lv_dropdown_set_dir(obj(), LV_DIR_RIGHT);
            break;
        case Symbol::UP:
            lv_dropdown_set_symbol(obj(), lvSymbol);
            lv_dropdown_set_dir(obj(), LV_DIR_BOTTOM);
            break;
        case Symbol::DOWN:
            lv_dropdown_set_symbol(obj(), lvSymbol);
            lv_dropdown_set_dir(obj(), LV_DIR_TOP);
            break;
    }
}

void DropDownList::setSelectAction(SelectCallback cb) {
    onSelect = cb;

    lv_obj_add_event_cb(obj(), [] (lv_event_t *e) {
        lv_obj_t *o = lv_event_get_target_obj(e);
        DropDownList *us = reinterpret_cast<DropDownList *>(lv_obj_get_user_data(o));
        us->onSelect();
    }, LV_EVENT_VALUE_CHANGED, nullptr);
}

void DropDownList::scrollUp() {
    if (! lv_obj_has_state(obj(), LV_STATE_FOCUSED)) {
        return;
    }
    uint32_t i = lv_dropdown_get_selected(obj());
    if (i > 0) {
        lv_dropdown_set_selected(obj(), i - 1);
    }
    else {
        lv_dropdown_set_selected(obj(), items - 1);
    }
}

void DropDownList::scrollDown() {
    if (! lv_obj_has_state(obj(), LV_STATE_FOCUSED)) {
        return;
    }
    uint32_t i = lv_dropdown_get_selected(obj());
    if (i == (items - 1)) {
        lv_dropdown_set_selected(obj(), 0);
    }
    else {
        lv_dropdown_set_selected(obj(), i + 1);
    }
}

} /* namespace avitab */

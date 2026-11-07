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
#include "TextArea.h"

namespace avitab {

TextArea::TextArea(WidgetPtr parent, const std::string& text, const int length, const bool multiLine):
    Widget(parent)
{
    lv_obj_t *ta = lv_textarea_create(parentObj());
    setObj(ta);
    setText(text);
    if (length) {
        setMaxLength(length);
    }
    if (! multiLine) {
        setMultiLine(multiLine);
    }
}

void TextArea::setMultiLine(bool multiLine) {
    if (multiLine) {
        lv_textarea_set_one_line(obj(), false);
    } else {
        lv_textarea_set_one_line(obj(), true);
    }
}

void TextArea::setMaxLength(int length) {
    if (length) {
        lv_textarea_set_max_length(obj(), length);
    }
}

void TextArea::setText(const std::string& text) {
    lv_textarea_set_text(obj(), text.c_str());
}

void TextArea::setPlaceholderText(const std::string& text) {
    lv_textarea_set_placeholder_text(obj(), text.c_str());
}

void TextArea::addText(const std::string& text) {
    lv_textarea_add_text(obj(), text.c_str());
}

std::string TextArea::getText() {
    return lv_textarea_get_text(obj());
}
/* FIXME
void TextArea::setShowCursor(bool show) {
    if (show) {
        lv_ta_set_cursor_type(obj(), LV_CURSOR_LINE);
    } else {
        lv_ta_set_cursor_type(obj(), static_cast<lv_cursor_type_t>(LV_CURSOR_LINE | LV_CURSOR_HIDDEN));
    }
}
*/
} /* namespace avitab */

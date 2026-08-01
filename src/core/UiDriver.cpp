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

#include "UiDriver.h"
#include "Logger.h"
#include <cstring>

namespace avitab {

void UiDriverBase::init(int width, int height) {
    logger::verbose("Initializing GUI driver");

    bufferWidth = width;
    bufferHeight = height;
    buffer.resize(width * height);
}

WindowRect UiDriverBase::getWindowRect() {
    return {};
}

void UiDriverBase::setResizeCallback(ResizeCallback cb) {
    onResize = cb;
}

void UiDriverBase::resize(int newWidth, int newHeight) {
    bufferWidth = newWidth;
    bufferHeight = newHeight;
    buffer.resize(bufferWidth * bufferHeight);
    if (onResize) {
        onResize(newWidth, newHeight);
    }
}

void UiDriverBase::createPanel(int left, int bottom, int width, int height, PanelControlMode mode) {
}

void UiDriverBase::hidePanel() {
}

void UiDriverBase::blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t* data) {
    if(x2 < 0 || y2 < 0 || x1 > bufferWidth - 1 || y1 > bufferHeight - 1) {
        return;
    }

    uint32_t *fb = reinterpret_cast<uint32_t *>(buffer.data());

    uint32_t w = x2 - x1 + 1;
    for (int32_t y = y1; y <= y2; y++) {
        memcpy(reinterpret_cast<void *>(fb + y * bufferWidth + x1),
               reinterpret_cast<const void *>(data),
               w * sizeof(uint32_t));
        data += w;
    }
}

int UiDriverBase::width() {
    return bufferWidth;
}

int UiDriverBase::height() {
    return bufferHeight;
}

uint32_t* UiDriverBase::data() {
    return buffer.data();
}

void UiDriverBase::setWantKeyInput(bool wantKeys) {
    if (enableKeyInput != wantKeys) {
        logger::verbose("Want key input: %d", wantKeys);
    }
    enableKeyInput = wantKeys;
}

bool UiDriverBase::wantsKeyInput() {
    return enableKeyInput;
}

void UiDriverBase::pushKeyInput(uint32_t c) {
    std::lock_guard<std::mutex> lock(keyMutex);
    if (enableKeyInput) {
        keyInput.push(c);
    }
}

uint32_t UiDriverBase::popKeyPress() {
    int res = 0;
    {
        std::lock_guard<std::mutex> lock(keyMutex);
        if (!keyInput.empty()) {
            res = keyInput.front();
            keyInput.pop();
        }
    }
    return res;
}

void UiDriverBase::passLeftClick(bool, bool) {
}

void UiDriverBase::passWheel(int) {
}

UiDriverBase::~UiDriverBase() {
    logger::verbose("Destroying GUI driver");
}

}

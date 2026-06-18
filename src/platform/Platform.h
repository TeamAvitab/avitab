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
#pragma once

#include <string>
#include <vector>
#include <cstdarg>
#include <chrono>
#include <fstream>
#include <filesystem>

namespace platform {

struct DirEntry {
    std::filesystem::path utf8Name;
    bool isDirectory;
};

enum class MediaControl {
    MEDIA_PAUSE,
    MEDIA_PREV,
    MEDIA_NEXT
};

enum class Platform {
    LINUX,
    WINDOWS,
    MAC
};

Platform getPlatform();

std::chrono::time_point<std::chrono::steady_clock> measureTime();
int getElapsedMillis(std::chrono::time_point<std::chrono::steady_clock> startAt);

std::wstring strToWstr(const std::string &str);

constexpr size_t getMaxPathLen();
std::filesystem::path getExectuablePath();
std::filesystem::path getFallbackPath();

std::string pathToDisplayString(const std::filesystem::path &utf8Path);
std::vector<DirEntry> readDirectory(const std::filesystem::path &utf8Path);

std::string getLocalTime(const std::string &format);
std::string getClipboardContent();

std::string formatStringArgs(const std::string format, va_list args);
std::string formatString(const std::string format, ...);

void controlMediaPlayer(MediaControl ctrl);
std::string lower(const std::string &in);
std::string upper(const std::string &in);

std::string getMachineID();

void openBrowser(const std::string &url);

}

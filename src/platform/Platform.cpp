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
#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <shellapi.h>
#else
#   include <unistd.h>
#   include <uuid/uuid.h>
#endif
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#include <locale>
#include <codecvt>
#include <ctime>
#include <sys/stat.h>
#include <cstdlib>
#include <climits>
#include <algorithm>
#include <regex>
#include <fstream>
#include <cstring>
#include <cassert>
#include "Platform.h"
#include "Logger.h"

/*
 * The purpose of this module is to put all platform (as in Posix or Win32)
 * specific functions into a single module so that the rest of the application
 * can stay 'clean'. It's mainly about file system related stuff: Windows
 * returns ANSI strings when using the POSIX wrapper for file system access,
 * but most libraries (such as mupdf) expect UTF8 strings when passing file
 * names.
 *
 * The idea is to use UTF8 in AviTab as well, so these wrapper functions
 * should always accept and return UTF8 strings.
 */

// The maximum length that WE support
#define AVITAB_PATH_LEN_MAX 2048

namespace platform {

platform::Platform getPlatform() {
#ifdef _WIN32
    return Platform::WINDOWS;
#elif defined __linux__
    return Platform::LINUX;
#elif defined __APPLE__
    return Platform::MAC;
#else
#   error "Unknown platform"
#endif
}

std::chrono::time_point<std::chrono::steady_clock> measureTime() {
    return std::chrono::steady_clock::now();
}

int getElapsedMillis(std::chrono::time_point<std::chrono::steady_clock> startAt) {
    auto endAt = measureTime();
    return std::chrono::duration_cast<std::chrono::milliseconds>(endAt - startAt).count();
}

constexpr size_t getMaxPathLen() {
    return AVITAB_PATH_LEN_MAX;
}

#if defined(_WIN32)
std::filesystem::path getExectuablePath() {
    wchar_t buf[AVITAB_PATH_LEN_MAX];
    DWORD size = AVITAB_PATH_LEN_MAX;
    HANDLE proc = GetCurrentProcess();
    QueryFullProcessImageNameW(proc, 0, buf, &size);

    auto u16str = std::wstring(buf);
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    auto u8str = convert.to_bytes(buf);
    auto exe = std::filesystem::path(u8str);
    return exe.parent_path();
}
#elif defined(__linux__)
std::filesystem::path getExectuablePath() {
    char dest[AVITAB_PATH_LEN_MAX];
    memset(dest,0,sizeof(dest));
    if (readlink("/proc/self/exe", dest, AVITAB_PATH_LEN_MAX) == -1) {
        return getFallbackPath();
    }
    auto exe = std::filesystem::u8path(dest);
    return std::filesystem::canonical(exe.parent_path());
}
#elif defined(__APPLE__)
std::filesystem::path getExectuablePath() {
    char path[AVITAB_PATH_LEN_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        return getFallbackPath();
    }
    auto exe = std::filesystem::u8path(path);
    return std::filesystem::canonical(exe.parent_path());
}
#endif

#if defined(_WIN32)
std::filesystem::path getFallbackPath() {
    auto h = std::filesystem::path(getenv("USERPROFILE"));
    if (!std::filesystem::exists(h)) {
        h = "C:\\";
    }
    return h;
}
#else
std::filesystem::path getFallbackPath() {
    auto h = std::filesystem::path(getenv("HOME"));
    if (!std::filesystem::exists(h)) {
        h = "/";
    }
    return h;
}
#endif

std::string pathToDisplayString(const std::filesystem::path &utf8Path) {
    static const char *hex = "0123456789abcdef";
    std::string a;
    auto u8s = utf8Path.u8string();
    auto i = u8s.begin();
    while (i != u8s.end()) {
        auto m = u8s.end() - i;
        uint32_t c = *i++ & 0xff;
        if (c < 0x80) {
            // simple ascii
            a.push_back(c);
        } else if (((c & 0xe0) == 0xc0) && (m >= 2)) {
            // 2-byte sequence
            i += 1;
            a.push_back('?');
        } else if (((c & 0xf0) == 0xe0) && (m >= 3)) {
            // 3-byte sequence
            i += 2;
            a.push_back('?');
        } else if (((c & 0xf0) == 0xf0) && (m >= 4)) {
            // 4-byte sequence
            i += 3;
            a.push_back('?');
        } else {
            // did we lose sync? bit weird, but let's pick it up again
            while (i != u8s.end()) {
                if ((*i & 0xc0) != 0x80)
                    break;
                ++i;
            }
        }
    }
    return a;
}

// This is only used by the TTFStamper for overlaying copyright messages
std::wstring strToWstr(const std::string& str) {
    size_t size = std::mbstowcs(nullptr, str.c_str(), 0); // Get required size
    std::wstring u16str(size, L'\0'); // Create wstring with required size
    std::mbstowcs(&u16str[0], str.c_str(), str.length() + 1); // Convert
    return u16str;
}

std::vector<DirEntry> readDirectory(const std::filesystem::path& utf8Path) {
    std::vector<DirEntry> entries;

#ifdef _WIN32
    // this fragment supports moving to other drives by adding extra
    // entries when browsing the current drive's root directory.
    if (utf8Path == utf8Path.root_path()) {
        auto ldbitmap = GetLogicalDrives();
        std::string drive("A:\\");
        while (ldbitmap) {
            if ((ldbitmap & 0b1) && (utf8Path.string() != drive)) {
                if (GetDiskFreeSpaceExA(drive.c_str(), NULL, NULL, NULL)) {
                    DirEntry entry;
                    entry.utf8Name = drive;
                    entry.isDirectory = true;
                    entries.push_back(entry);
                }
            }
            ldbitmap >>= 1;
            ++drive[0];
        }
    }
#endif

    for (auto &e: std::filesystem::directory_iterator(utf8Path)) {
        auto name = e.path().filename().u8string();

        if (name.empty() || name[0] == '.') {
            continue;
        }

        DirEntry entry;
        entry.utf8Name = e.path().filename();
        entry.isDirectory = e.is_directory();
        entries.push_back(entry);
    }

    return entries;
}

std::string getLocalTime(const std::string &format) {
    time_t now = time(nullptr);
    tm *local = localtime(&now);

    char buf[16];
    strftime(buf, sizeof(buf), format.c_str(), local);
    return buf;
}

#ifdef _WIN32
std::string getClipboardContent() {
    if (!OpenClipboard(nullptr)) {
        return "No clipboard access";
    }

    std::string res;
    HANDLE data = GetClipboardData(CF_UNICODETEXT);
    if (data) {
        WCHAR *text = (WCHAR *) GlobalLock(data);
        if (text) {
            size_t utf16len = wcslen(text);
            size_t utf8len = WideCharToMultiByte(CP_UTF8, 0, text, utf16len, nullptr, 0, nullptr, nullptr);
            res.reserve(utf8len + 1);
            WideCharToMultiByte(CP_UTF8, 0, text, utf16len, &res[0], utf8len, nullptr, nullptr);
            res[utf8len] = '\0';
            GlobalUnlock(data);
        }
    }

    CloseClipboard();

    return res;
}
#else
std::string getClipboardContent() {
    return "Not supported on your platform";
}
#endif

std::string formatStringArgs(const std::string format, va_list list) {
    char buf[2048];
    size_t max_len = sizeof buf;
    vsnprintf(buf, max_len, format.c_str(), list);
    return buf;
}

std::string formatString(const std::string format, ...) {
    va_list args;
    va_start(args, format);
    std::string formatted = formatStringArgs(format, args);
    va_end(args);

    return formatted;
}

void controlMediaPlayer(MediaControl ctrl) {
#ifdef _WIN32
    uint8_t key = 0;
    switch (ctrl) {
    case MediaControl::MEDIA_PAUSE: key = VK_MEDIA_PLAY_PAUSE; break;
    case MediaControl::MEDIA_NEXT:  key = VK_MEDIA_NEXT_TRACK; break;
    case MediaControl::MEDIA_PREV:  key = VK_MEDIA_PREV_TRACK; break;
    }
    UINT scanCode = MapVirtualKeyA(key, 0);

    keybd_event(key, scanCode, KEYEVENTF_EXTENDEDKEY, 0);
    keybd_event(key, scanCode, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
#endif
}

std::string lower(const std::string& in) {
    std::string res;
    std::transform(in.begin(), in.end(), std::back_inserter(res), ::tolower);
    return res;
}

std::string upper(const std::string& in) {
    std::string res;
    std::transform(in.begin(), in.end(), std::back_inserter(res), ::toupper);
    return res;
}

std::string getMachineID() {
#ifdef _WIN32
    char buf[255];
    DWORD bufSiz = sizeof(buf);
    RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", "MachineGuid", RRF_RT_REG_SZ, nullptr, buf, &bufSiz);
    return std::string(buf);
#elif defined(__linux__)
    std::ifstream idStream("/var/lib/dbus/machine-id");
    std::string id;
    idStream >> id;
    return id;
#elif defined(__APPLE__)
    uuid_t id;
    timespec wait;
    wait.tv_nsec = 0;
    wait.tv_sec = 0;
    gethostuuid(id, &wait);
    char buf[48];
    uuid_unparse(id, buf);
    return std::string(buf);
#endif
}

void openBrowser(const std::string& url) {
    logger::info("Opening browser: %s", url.c_str());
#ifdef _WIN32
    ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    std::string cmd = std::string("open \"") + url + "\"";
    system(cmd.c_str());
#else
    std::string browser_cmd = std::string("xdg-open '") + url + "'";
    auto res = system(browser_cmd.c_str());
    if (res != 0) {
        logger::warn("Couldn't launch browser: %d", res);
    }
#endif
}

}

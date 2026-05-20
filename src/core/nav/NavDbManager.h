/* This file is part of the Avitab project. See the README and LICENSE for details. */

#pragma once
#include <atomic>
#include <filesystem>
#include <list>
#include <mutex>
#include <thread>
#include <atomic>
#include "Navigation.h"

namespace navdb {

// extends the read-only NavDatabase interface to add functions for the
// NavDbManager to append user fixes (nav source independent) and to support
// hot-swapping of active database.

class LoadableNavDb : public NavDatabase
{
public:
    virtual void setReloadStarted() = 0;
    virtual void setReloadCancelled() = 0;
    virtual void setReplacement(std::shared_ptr<NavDatabase> latest) = 0;
};

//

// one of these is created at startup to provide a nav database, and if necessary
// build an updated one to replace it.

class NavDbManager
{
public:
    NavDbManager(const std::filesystem::path & xplaneDataRoot, const std::filesystem::path & msfsDataRoot,
                    bool excludeXPlane = false, bool excludeMSFS = true, bool preferMSFS = false);
    ~NavDbManager();

    std::shared_ptr<NavDatabase> getNavDatabase();

    void loadUserFixes(const std::filesystem::path filename);

    void stop();
    bool stopRequested() const { return shouldStop; }

private:
    struct FileInfo {
        std::filesystem::path fPath;
        std::uintmax_t fSize;
        std::filesystem::file_time_type lastWrite;
    };

private:
    void backgroundLoader();
    std::list<FileInfo> loadPreviousSqlDatabase();
    bool getNavSourceFilesList(std::list<FileInfo> &files);
    std::list<FileInfo> getXplaneNavSourceFilesList();
    std::list<FileInfo> getMsfsNavSourceFilesList();
    bool equalSigs(const std::list<FileInfo> &s1, const std::list<FileInfo> &s2);
    std::shared_ptr<LoadableNavDb> buildXplaneDatabase(const std::list<FileInfo> &fl);
    std::shared_ptr<LoadableNavDb> buildMsfsDatabase(const std::list<FileInfo> &fl);
    void copyUserFixes();
    void makeActive(std::shared_ptr<LoadableNavDb> latest);

private:
    const std::filesystem::path xplaneDataRoot;
    const std::filesystem::path msfsDataRoot;
    const bool excludeXPlane, excludeMSFS, preferMSFS;
    std::shared_ptr<LoadableNavDb> activeNavDB;
    std::vector<std::shared_ptr<UserFix>> userFixes;
    std::mutex mutex;
    std::unique_ptr<std::thread> worker;
    std::atomic_bool shouldStop;

};

}

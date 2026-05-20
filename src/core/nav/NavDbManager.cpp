/* This file is part of the Avitab project. See the README and LICENSE for details. */

#include <cassert>
#include "NavDbManager.h"
#include "InMemNavDb.h"
#include "loaders/xplane/NavLoaderXp.h"
#include "loaders/UserFixLoader.h"
#include "Logger.h"

namespace navdb {

NavDbManager::NavDbManager(const std::filesystem::path & x, const std::filesystem::path & m,
                            bool exXP, bool exMSFS, bool favMSFS)
:   xplaneDataRoot(x), msfsDataRoot(m),
    excludeXPlane(exXP), excludeMSFS(exMSFS), preferMSFS(favMSFS),
    shouldStop(false)
{
    assert(!(excludeXPlane && excludeMSFS));

    // initially create an empty nav db as the active one
    activeNavDB = std::make_shared<InMemoryNavDb>();

    // start the background worker, this will replace the empty nav db once the new one is ready
    worker = std::make_unique<std::thread>(&NavDbManager::backgroundLoader, this);
}

NavDbManager::~NavDbManager()
{
    stop();
}

std::shared_ptr<NavDatabase> NavDbManager::getNavDatabase()
{
    std::lock_guard<std::mutex> lock(mutex);
    return activeNavDB;
}

void NavDbManager::loadUserFixes(const std::filesystem::path filename)
{
    if (filename.empty()) return;
    if (!std::filesystem::exists(filename)) {
        logger::warn("No such user fixes file: %s", filename.string().c_str());
        return;
    }
    try {
        UserFixLoader loader;
        userFixes = loader.load(filename);
        copyUserFixes();
    } catch (...) {
        logger::error("Failed to load user fixes: %s", filename.string().c_str());
    }
}

void NavDbManager::copyUserFixes()
{
    std::lock_guard<std::mutex> lock(mutex);
    activeNavDB->clearUserFixes();
    for (auto &uf : userFixes) {
        activeNavDB->addUserFix(uf);
    }
}

void NavDbManager::stop() {
    if (worker) {
        shouldStop = true;
        worker->join();
        worker.reset();
    }
}

void NavDbManager::backgroundLoader()
{
    // runs in background (async or thread)

    // 1) try loading the SQL-backed nav database. returns the list of files and filestats that were used to cretate the database.
    auto sqlDbSigs = loadPreviousSqlDatabase();

    // 2) get a list of files to use and their filestats (based on which sims are preferred)
    std::list<FileInfo> fileSigs;
    auto isMsfsNav = getNavSourceFilesList(fileSigs);

    // 3) if the filelists correspond then stop.
    if (equalSigs(sqlDbSigs, fileSigs)) return;
    
    // 4) set the reloading flag in the active database
    activeNavDB->setReloadStarted();
    
    // 5) build a new database from the filelist
    auto latest = isMsfsNav ? buildMsfsDatabase(fileSigs) : buildXplaneDatabase(fileSigs);

    // 6) swap the new db into the active place, and set the superseded flag in the old one
    if (latest) {
        makeActive(latest);
        copyUserFixes();
    } else {
        activeNavDB->setReloadCancelled();
    }

    // we will only do one background update, so the background loader is now finished
}

std::list<NavDbManager::FileInfo> NavDbManager::loadPreviousSqlDatabase()
{
    // TODO-SQL - revive this eventually
    // TODO-SQL - makeActive(sqlDB); copyUserFixes();
    return std::list<FileInfo>();
}

bool NavDbManager::getNavSourceFilesList(std::list<FileInfo> &files)
{
    if (excludeXPlane || (preferMSFS && !excludeMSFS)) {
        files = getMsfsNavSourceFilesList();
        return true;
    }
    files = getXplaneNavSourceFilesList();
    return false;
}

std::list<NavDbManager::FileInfo> NavDbManager::getXplaneNavSourceFilesList()
{
    // TODO-SQL - this needs to be properly implemented.
    // for now it returns a phony file that will always trigger a rescan.
    FileInfo phony;
    phony.fPath = "./xp-phony";
    phony.fSize = rand();
    std::list<FileInfo> fl;
    fl.push_back(phony);
    return fl;
}

std::list<NavDbManager::FileInfo> NavDbManager::getMsfsNavSourceFilesList() {
    // TODO-SQL - awaiting implementation
    return std::list<FileInfo>();
}

bool NavDbManager::equalSigs(const std::list<FileInfo> &s1, const std::list<FileInfo> &s2)
{
    auto i1 = s1.begin();
    auto i2 = s2.begin();
    while ((i1 != s1.end()) && (i2 != s2.end())) {
        if (i1->fPath != i2->fPath) return false;
        if (i1->fSize != i2->fSize) return false;
        if (i1->lastWrite != i2->lastWrite) return false;
        ++i1; ++i2;
    }
    return (i1 == s1.end()) && (i2 == s2.end());
}

std::shared_ptr<LoadableNavDb> NavDbManager::buildXplaneDatabase(const std::list<FileInfo> &fl)
{
    try {
        auto xpnl = std::make_unique<xdata::XplaneNavDataLoader>(*this, xplaneDataRoot);
        xpnl->discoverSceneries();
        xpnl->load();
        return xpnl->getDatabase();
    } catch (std::exception& e) {
        logger::error("Failed to load X-Plane NAV data: %s", e.what());
    }
    return nullptr;
}

std::shared_ptr<LoadableNavDb> NavDbManager::buildMsfsDatabase(const std::list<FileInfo> &fl)
{
    return nullptr;
}

void NavDbManager::makeActive(std::shared_ptr<LoadableNavDb> latest)
{
    assert(latest);
    std::lock_guard<std::mutex> lock(mutex);
    activeNavDB->setReplacement(latest);
    activeNavDB = latest;
}

}

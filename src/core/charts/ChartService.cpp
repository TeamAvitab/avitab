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

#include "ChartService.h"
#include "platform/CrashHandler.h"
#include "Logger.h"
#include "platform/Platform.h"
#include "charts/Crypto.h"
#include <nlohmann/json.hpp>
#include "UnzipWrapper.h"


namespace apis {

ChartService::ChartService(const std::filesystem::path &programPath, const std::vector<std::string> remote_georefs_urls) {
    try {
        navigraph = std::make_shared<navigraph::NavigraphAPI>(programPath / "Navigraph");
        useNavigraph = true;
    } catch (...) {
        logger::error("Exception thrown during Navigraph service construction");
    }

    if (chartfox::ChartFoxAPI::isSupported()) {
        try {
            chartFox = std::make_shared<chartfox::ChartFoxAPI>(programPath / "ChartFox");
            useChartFox = chartFox->isAuthenticated();
        } catch (...) {
            logger::error("Exception thrown during ChartFox service construction");
        }
    }
    localFile= std::make_shared<localfile::LocalFileAPI>(programPath / "charts");

    keepAlive = true;
    apiThread = std::make_unique<std::thread>(&ChartService::workLoop, this);

    auto calibrationPath = programPath / "MapTiles" / "Mercator" / "Calibration";
    for (const auto& remote_georefs_url : remote_georefs_urls) {
        try {
	    loadTeamAvitabGeorefs(calibrationPath, remote_georefs_url);
        } catch (...) {
            logger::warn("Unable to download georefs from %s", remote_georefs_url.c_str());
	}
    }

    if (std::filesystem::exists(calibrationPath)) {
        scanJsonFiles(calibrationPath);
        logger::info("Found %d calibration files", jsonFileHashes.size());
    } else {
        logger::info("Calibration folder does not exist: %s", calibrationPath.string().c_str());
    }
}

std::vector<uint8_t> ChartService::downloadGeorefZip(const std::string &remote_georefs_url) {
    if (remote_georefs_url.empty()) {
        logger::info("remote_georefs_url empty, not downloading TeamAvitab georefs");
        return {};
    }

    bool cancelToken = false;
    auto zipBlob = downloader.download(remote_georefs_url, cancelToken);
    if (zipBlob.empty()) {
        logger::error("Downloaded zip blob is empty or download failed for TeamAvitabGeorefs.");
        return {};
    }

    constexpr size_t MAX_ALLOWED_ZIP_SIZE = 16 * 1024 * 1024; 
    if (zipBlob.size() > MAX_ALLOWED_ZIP_SIZE) {
        logger::error("Downloaded zip blob exceeds maximum allowed safety size (%d bytes).", zipBlob.size());
        return {};
    }

    logger::info("Downloaded TeamAvitab georef data %d bytes", zipBlob.size());
    return zipBlob;
}

bool ChartService::prepareCalibrationDirectory(const std::filesystem::path &calibrationPath) {
    std::error_code e;
    if (!std::filesystem::exists(calibrationPath)) {
        std::filesystem::create_directories(calibrationPath, e);
        if (e) {
            logger::error("Failed to create calibration directory: %s", e.message().c_str());
            return false;
        }
    }
    return true;
}

bool ChartService::writeZipBlobToFile(const std::filesystem::path &zipPath, const std::vector<uint8_t> &zipBlob) {
    std::ofstream stream(zipPath, std::ios::out | std::ios::binary);
    if (!stream.is_open()) {
        logger::error("Failed to open file for writing: %s", zipPath.string().c_str());
        return false;
    }

    stream.write(reinterpret_cast<const char *>(zipBlob.data()), zipBlob.size());
    if (stream.bad() || !stream.good()) {
        logger::error("Error occurred while writing to file: %s", zipPath.string().c_str());
        stream.close();
        std::error_code e;
        std::filesystem::remove(zipPath, e); // Clean up the corrupted/partial file
        return false;
    }
    stream.close();
    return true;
}

void ChartService::extractGeorefZip(const std::filesystem::path &zipPath, const std::filesystem::path &zipExtractPath) {
    try {
        std::filesystem::remove_all(zipPath);
        std::filesystem::remove_all(zipExtractPath);
    } catch (...) {
        logger::warn("Unable to remove existing directories");
    }

    logger::info("Extracting into %s", zipExtractPath.string().c_str());
    std::error_code e;
    std::filesystem::create_directories(zipPath.parent_path(), e);
}

void ChartService::unzipGeorefData(const std::filesystem::path &zipPath, const std::filesystem::path &zipExtractPath) {
    try {
        unzip::unzip_file(zipPath.string().c_str(), zipExtractPath.string().c_str());
    } catch (const std::exception &e) {
        logger::error("Failed to unzip georef data: %s", e.what());
    } catch (...) {
        logger::error("Unknown error occurred during unzip execution.");
    }
}

void ChartService::loadTeamAvitabGeorefs(const std::filesystem::path &calibrationPath, const std::string remote_georefs_url) {
    if (remote_georefs_url.empty()) {
        std::error_code e;
        std::filesystem::remove_all(calibrationPath / "TeamAvitabGeorefs", e);
        std::filesystem::remove(calibrationPath / "TeamAvitabGeorefs.zip", e);
        logger::info("remote_georefs_url empty, ensure TeamAvitabGeorefs files removed");
    }

    auto zipBlob = downloadGeorefZip(remote_georefs_url);
    if (zipBlob.empty()) return;

    if (!prepareCalibrationDirectory(calibrationPath)) return;

    auto zipName = std::filesystem::path(remote_georefs_url).filename();
    auto zipPath = calibrationPath / zipName;
    std::string remoteHash = crypto.sha256String(zipBlob);

    if (std::filesystem::exists(zipPath) && (crypto.getFileSha256(zipPath) == remoteHash)) {
        logger::info("Local and remote TeamAvitab georef zips same, no update required");
        return;
    }

    auto zipExtractPath = zipPath;
    zipExtractPath.replace_extension("");

    extractGeorefZip(zipPath, zipExtractPath);
    if (!writeZipBlobToFile(zipPath, zipBlob)) return;

    unzipGeorefData(zipPath, zipExtractPath);
}

void ChartService::setUseNavigraph(bool use) {
    useNavigraph = use;
}

void ChartService::setUseChartFox(bool use) {
    useChartFox = chartFox ? use : false;
}

std::shared_ptr<navigraph::NavigraphAPI> ChartService::getNavigraph() {
    return navigraph;
}

std::shared_ptr<chartfox::ChartFoxAPI> ChartService::getChartFox() {
    return chartFox;
}

std::shared_ptr<APICall<bool>> ChartService::loginNavigraph() {
    auto call = std::make_shared<APICall<bool>>([this] {
        if (useNavigraph) {
            navigraph->init();
        }

        return true;
    });
    return call;
}

std::shared_ptr<APICall<bool>> ChartService::verifyChartFoxAccess() {
    auto call = std::make_shared<APICall<bool>>([this] {
        if (chartFox) return chartFox->isAuthenticated();
        return false;
    });
    return call;
}

std::shared_ptr<APICall<ChartService::ChartList>> ChartService::getChartsFor(const std::string &icao) {
    auto call = std::make_shared<APICall<ChartList>>([this, icao] {
        ChartList res;

        if (useNavigraph && navigraph->hasChartsFor(icao)) {
            auto charts = navigraph->getChartsFor(icao);
            res.insert(res.end(), charts.begin(), charts.end());
        }

        if (useChartFox) {
            auto charts = chartFox->getChartsFor(icao);
            res.insert(res.end(), charts.begin(), charts.end());
        }

        if (useLocalFile) {
            auto charts = localFile->getChartsFor(icao);
            res.insert(res.end(), charts.begin(), charts.end());
        }

        return res;
    });

    return call;
}

std::shared_ptr<APICall<std::shared_ptr<Chart>>> ChartService::loadChart(std::shared_ptr<Chart> chart) {
    auto call = std::make_shared<APICall<std::shared_ptr<Chart>>>([this, chart] {
        auto bgChart = std::dynamic_pointer_cast<chartfox::ChartFoxChart>(chart);
        if (bgChart) {
            chartFox->loadChart(bgChart);
            auto blob = bgChart->getChartData();
            auto hash = crypto.sha256String(blob);
            std::string localCalibrationMetadata = getCalibrationMetadataForHash(hash);
            if (localCalibrationMetadata != "") {
                // Use local calibration metadata, overriding any Chartfox georef
                bgChart->setCalibrationMetadata(localCalibrationMetadata);
            }
        }

        auto nvChart = std::dynamic_pointer_cast<navigraph::NavigraphChart>(chart);
        if (nvChart) {
            navigraph->loadChartImages(nvChart);
        }

        auto lfChart = std::dynamic_pointer_cast<localfile::LocalFileChart>(chart);
        if (lfChart) {
            localFile->loadChart(lfChart);
            std::string cm = getCalibrationMetadataForFile(lfChart->getPath());
            lfChart->setCalibrationMetadata(cm);
        }

        return chart;
    });

    return call;
}

std::shared_ptr<APICall<std::string>> ChartService::getChartFoxDonationLink() {
    auto call = std::make_shared<APICall<std::string>>([this] {
        return chartFox->getDonationLink();
    });

    return call;
}

void ChartService::submitCall(std::shared_ptr<BaseCall> call) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!keepAlive) {
        return;
    }
    pendingCalls.push_back(call);
    workCondition.notify_one();
}

bool ChartService::hasWork() {
    // gets called with locked mutex
    if (!keepAlive) {
        return true;
    }

    return !pendingCalls.empty();
}

void ChartService::workLoop() {
    crash::ThreadCookie crashCookie;

    while (keepAlive) {
        using namespace std::chrono_literals;

        std::unique_lock<std::mutex> lock(mutex);
        workCondition.wait_for(lock, std::chrono::seconds(1), [this] () { return hasWork(); });

        if (!keepAlive) {
            break;
        }

        // create copy to work on while locked so we can work unlocked
        std::vector<std::shared_ptr<BaseCall>> callsCopy;
        std::swap(callsCopy, pendingCalls);
        lock.unlock();

        for (auto call: callsCopy) {
            try {
                call->exec();
            } catch (const std::exception &e) {
                logger::warn("Oof! Uncaught exception in charts API: %s", e.what());
            }
        }
    }
}

void ChartService::stop() {
    if (apiThread) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            keepAlive = false;
            pendingCalls.clear();
            workCondition.notify_one();
        }
        apiThread->join();
        apiThread.reset();
    }
}

void ChartService::scanJsonFiles(std::filesystem::path dir) {
    auto items = platform::readDirectory(dir);
    for (auto &entry: items) {
        auto fullPath = dir / entry.utf8Name;
        if (entry.isDirectory) {
            // Recurse
            scanJsonFiles(fullPath);
        } else if (entry.utf8Name.u8string().find(".json") != std::string::npos) {
            std::ifstream jsonFile(fullPath);
            if (jsonFile.fail()) {
                continue;
            }
            std::string jsonStr((std::istreambuf_iterator<char>(jsonFile)),
                                 std::istreambuf_iterator<char>());
            nlohmann::json json = nlohmann::json::parse(jsonStr);
            std::string hash = json.value("/calibration/hash"_json_pointer, "");
            if (hash.length() == 64) {
                if (jsonFileHashes.count(hash) == 1) {
                    LOG_INFO(0, "Duplicate hash %s", hash.c_str());
                    LOG_INFO(0, " %s", fullPath.c_str());
                    LOG_INFO(0, " %s", jsonFileHashes[hash].c_str());
                }
                jsonFileHashes[hash] = fullPath;
            }
        }
    }
}

std::string ChartService::getCalibrationMetadataForFile(std::filesystem::path utf8ChartFileName) const {
    std::string hash = crypto.getFileSha256(utf8ChartFileName);
    return getCalibrationMetadataForHash(hash);
}

std::string ChartService::getCalibrationMetadataForHash(std::string hash) const {
    if (jsonFileHashes.count(hash) == 1) {
        auto calibrationFilename = jsonFileHashes.at(hash);
        std::ifstream hashedJsonFile(calibrationFilename);
        if (hashedJsonFile.good()) {
            std::string jsonStr((std::istreambuf_iterator<char>(hashedJsonFile)),
                                 std::istreambuf_iterator<char>());
            logger::info("Found hash-matched calibration file");
            logger::info(" at '%s'", calibrationFilename.u8string().c_str());
            logger::info(" with sha256 %s", hash.c_str());
            return jsonStr;
        }
    }

    logger::info("No hash-matched calibration file for sha256");
    logger::info(" %s", hash.c_str());
    return "";
}

ChartService::~ChartService() {
    stop();
}

}

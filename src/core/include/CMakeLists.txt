# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    AviTabCore.h
    AviTabSimDriver.h
    AviTabWinDriver.h
    JsonConfig.h
    Logger.h
    WorldGeometry.h

    Settings.h # REFACTOR - needs some abstraction
    # REFACTOR - these 2 files should not need exposing to non-core sources
    AirportConfig.h
    OverlayConfig.h
)

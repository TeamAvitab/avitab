# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    AviTabCore.h
    AviTabSimServices.h
    AviTabUiServices.h
    JsonConfig.h
    Logger.h
    Navigation.h
    WorldGeometry.h

    SimDriver.h
    UiDriver.h

    Settings.h # REFACTOR - needs some abstraction
    # REFACTOR - these 2 files will be wrapped into the settings API
    AirportConfig.h
    OverlayConfig.h
)

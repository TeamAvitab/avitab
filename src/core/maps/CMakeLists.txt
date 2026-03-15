# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    Downloader.cpp
    Downloader.h
    OverlayConfig.h
    OverlayedAirport.cpp
    OverlayedAirport.h
    OverlayedDME.cpp
    OverlayedDME.h
    OverlayedFix.cpp
    OverlayedFix.h
    OverlayedILSLocalizer.cpp
    OverlayedILSLocalizer.h
    OverlayedMap.cpp
    OverlayedMap.h
    OverlayedNDB.cpp
    OverlayedNDB.h
    OverlayedNode.cpp
    OverlayedNode.h
    OverlayedRoute.cpp
    OverlayedRoute.h
    OverlayedUserFix.cpp
    OverlayedUserFix.h
    OverlayedVOR.cpp
    OverlayedVOR.h
    OverlayedWaypoint.cpp
    OverlayedWaypoint.h
    OverlayHelper.h
    OverlayHighlight.cpp
    OverlayHighlight.h
)

add_subdirectory(sources)

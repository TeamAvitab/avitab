# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    About.cpp
    About.h
    AirportApp.cpp
    AirportApp.h
    App.cpp
    App.h
    AppFunctions.h
    AppLauncher.cpp
    AppLauncher.h
    ChartsApp.cpp
    ChartsApp.h
    DocumentsApp.cpp
    DocumentsApp.h
    HeaderApp.cpp
    HeaderApp.h
    MapApp.cpp
    MapApp.h
    NotesApp.cpp
    NotesApp.h
    PlaneManualApp.cpp
    PlaneManualApp.h
    ProvidersApp.cpp
    ProvidersApp.h
    RouteApp.cpp
    RouteApp.h
)

add_subdirectory(components)

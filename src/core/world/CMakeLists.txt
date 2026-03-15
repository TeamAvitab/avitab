# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    LoadManager.cpp
    LoadManager.h
    World.h
)

add_subdirectory(graph)
add_subdirectory(loaders)
add_subdirectory(models)
add_subdirectory(parsers)
add_subdirectory(routing)

# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    World.h
)

add_subdirectory(graph)
add_subdirectory(models)
add_subdirectory(routing)

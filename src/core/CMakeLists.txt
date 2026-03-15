# This file is part of the Avitab project. See the README and LICENSE for details.

# Temporary to satisfy cmake
target_sources(avitab_core_lib PRIVATE
    Logger.cpp
    Logger.h
)

add_subdirectory(avitab)
add_subdirectory(charts)
add_subdirectory(env)
add_subdirectory(gui_toolkit)
add_subdirectory(libimg)
add_subdirectory(libnavsql)
add_subdirectory(maps)
add_subdirectory(world)

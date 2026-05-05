# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    AviTab.cpp
    AviTab.h
)

add_subdirectory(include)
add_subdirectory(apps)
add_subdirectory(charts)
add_subdirectory(gui_toolkit)
add_subdirectory(libimg)
add_subdirectory(libnavsql)
add_subdirectory(maps)
add_subdirectory(utils)
add_subdirectory(world)

# REFACTOR - these files need abstract interfaces to untangle from the core library
add_subdirectory(hybrids_to_refactor)

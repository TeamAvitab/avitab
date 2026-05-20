# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    AviTab.cpp
    Environment.cpp
    GUIDriver.cpp
)

add_subdirectory(include)
add_subdirectory(apps)
add_subdirectory(charts)
add_subdirectory(gui)
add_subdirectory(image)
add_subdirectory(maps)
add_subdirectory(nav)
add_subdirectory(other)

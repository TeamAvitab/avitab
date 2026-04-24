# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_xplane_lib PRIVATE
    XData.cpp
    XData.h
    XWorld.cpp
    XWorld.h
)

add_subdirectory(loaders)
add_subdirectory(models)
add_subdirectory(parsers)

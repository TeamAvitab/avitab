# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    Airport.cpp
    Airport.h
    Heliport.cpp
    Heliport.h
    Runway.cpp
    Runway.h
)

add_subdirectory(procs)

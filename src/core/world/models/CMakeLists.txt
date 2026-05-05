# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    Airway.cpp
    Airway.h
    Frequency.cpp
    Frequency.h
    Region.cpp
    Region.h
)

add_subdirectory(airport)
add_subdirectory(navaids)

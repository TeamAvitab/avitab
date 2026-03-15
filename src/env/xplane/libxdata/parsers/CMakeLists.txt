# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_xplane_lib PRIVATE
    AirportParser.cpp
    AirportParser.h
    AirwayParser.cpp
    AirwayParser.h
    CIFPParser.cpp
    CIFPParser.h
    CustomSceneryParser.cpp
    CustomSceneryParser.h
    FixParser.cpp
    FixParser.h
    MetarParser.cpp
    MetarParser.h
    NavaidParser.cpp
    NavaidParser.h
)

add_subdirectory(objects)

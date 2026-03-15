# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    BaseParser.cpp
    BaseParser.h
    FMSParser.cpp
    FMSParser.h
    UserFixParser.cpp
    UserFixParser.h
)

add_subdirectory(objects)

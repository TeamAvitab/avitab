# This file is part of the Avitab project. See the README and LICENSE for details.

# Avitab custom CMake script since Gumbo does not provide one.

# Project name and a few useful settings. Other commands can pick up the results
project(GUMBO
    VERSION 0.10.1
    DESCRIPTION "A library for html5 parsing"
    LANGUAGES C
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Threads REQUIRED)
include(Prefixer)

# The GUMBO library
set(GUMBO_SOURCES
    string_buffer.c
    util.c
    vector.c
    attribute.c
    char_ref.c
    utf8.c
    error.c
    tag.c
    parser.c
    tokenizer.c
    string_piece.c
)
PrefixSources(GUMBO_SOURCES "${AVITAB_gumbo_SOURCE_DIR}/src")

add_library(gumbo STATIC ${GUMBO_SOURCES})
target_include_directories(gumbo
    PUBLIC "${AVITAB_gumbo_SOURCE_DIR}/src"
)
if(MSVC)
    target_include_directories(gumbo PRIVATE "${AVITAB_gumbo_SOURCE_DIR}/visualc/include")
endif()

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_gumbo_PKGLIBS gumbo CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_gumbo_LOCATION "${AVITAB_LIBS3RD_DIR}/gumbo.lib" CACHE INTERNAL "")
else()
    set(AVITAB_gumbo_LOCATION "${AVITAB_LIBS3RD_DIR}/libgumbo.a" CACHE INTERNAL "")
endif()

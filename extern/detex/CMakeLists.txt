# This file is part of the Avitab project. See the README and LICENSE for details.

# This file is a basic CMake wrapper for the detex external library, which does not
# provide CMake support natively.

project(detex
    VERSION 0.1.2
    DESCRIPTION "A library for manipulating texture blocks"
    LANGUAGES C
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Threads REQUIRED)
include(Prefixer)

# The detex library
set(detex_SOURCES
    bits.c
    bits.h
    bptc-tables.c
    bptc-tables.h
    clamp.c
    convert.c
    dds.c
    decompress-bc.c
    decompress-bptc-float.c
    decompress-bptc.c
    decompress-eac.c
    decompress-etc.c
    decompress-rgtc.c
    detex.h
    division-tables.c
    file-info.c
    file-info.h
    half-float.c
    half-float.h
    hdr.c
    hdr.h
    ktx.c
    misc.c
    misc.h
    raw.c
    texture.c
)
PrefixSources(detex_SOURCES "${AVITAB_detex_SOURCE_DIR}")

add_library(detex STATIC ${detex_SOURCES})
target_include_directories(detex
    PUBLIC "${AVITAB_detex_SOURCE_DIR}"
)

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_detex_PKGLIBS detex CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_detex_LOCATION "${AVITAB_LIBS3RD_DIR}/detex.lib" CACHE INTERNAL "")
else()
    set(AVITAB_detex_LOCATION "${AVITAB_LIBS3RD_DIR}/libdetex.a" CACHE INTERNAL "")
endif()
set(AVITAB_detex_INCDIRS "${AVITAB_detex_SOURCE_DIR}" CACHE INTERNAL "")

# This file is part of the Avitab project. See the README and LICENSE for details.

# Avitab uses ZLib's CMake scripts.

set(ZLIB_BUILD_EXAMPLES OFF)

add_subdirectory("${AVITAB_zlib_SOURCE_DIR}" "${AVITAB_zlib_BINARY_DIR}" EXCLUDE_FROM_ALL)
add_library(ZLIB::ZLIB ALIAS zlibstatic)
add_library(zlibs ALIAS zlibstatic)

# These settings influence findPackage into detecting Avitab's version of ZLib.
# The name zlibs also avoids a clash in VS's generator.
set(ZLIB_USE_STATIC_LIBS ON CACHE INTERNAL "")
set(ZLIB_INCLUDE_DIR "${AVITAB_zlib_SOURCE_DIR}" "${AVITAB_zlib_BINARY_DIR}" CACHE INTERNAL "")
if(MSVC)
    set(ZLIB_LIBRARY "${AVITAB_LIBS3RD_DIR}/zlibs.lib" CACHE INTERNAL "")
else()
    set(ZLIB_LIBRARY "${AVITAB_LIBS3RD_DIR}/libzlibs.a" CACHE INTERNAL "")
endif()

# Cache settings to be used when BUILD_AVITAB_THIRDPARTY is OFF
set(AVITAB_zlib_PKGLIBS zlibs CACHE INTERNAL "")
set(AVITAB_zlibs_LOCATION "${ZLIB_LIBRARY}" CACHE INTERNAL "")

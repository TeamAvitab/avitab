# This file is part of the Avitab project. See the README and LICENSE for details.

# Avitab uses Zstd's CMake scripts.

set(ZSTD_BUILD_PROGRAMS OFF)
set(ZSTD_LEGACY_SUPPORT OFF)
set(ZSTD_BUILD_SHARED OFF)
set(ZSTD_BUILD_DICTBUILDER OFF)
set(ZSTD_USE_STATIC_RUNTIME ON)

add_subdirectory("${AVITAB_zstd_SOURCE_DIR}/build/cmake" "${AVITAB_zstd_BINARY_DIR}" EXCLUDE_FROM_ALL)
add_library(zstd ALIAS libzstd_static)

# Ensure that find_package selects Avitab's downloaded zstd static library 
# CMake target is called 'libzstd'
set(ZSTD_INCLUDE_DIR "${AVITAB_zstd_SOURCE_DIR}/lib" CACHE INTERNAL "")
if(MSVC)
    set(ZSTD_LIBRARY "${AVITAB_LIBS3RD_DIR}/zstd.lib" CACHE INTERNAL "")
else()
    set(ZSTD_LIBRARY "${AVITAB_LIBS3RD_DIR}/libzstd.a" CACHE INTERNAL "")
endif()

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_zstd_PKGLIBS zstd CACHE INTERNAL "")
set(AVITAB_zstd_LOCATION "${ZSTD_LIBRARY}" CACHE INTERNAL "")

# This file is part of the Avitab project. See the README and LICENSE for details.

set(PNG_STATIC ON)
set(PNG_SHARED OFF)
if(APPLE)
    set(PNG_FRAMEWORK OFF)
    # libpng doesn't support h/w optimization for universal binaries in one pass
    # turn it off until we figure out a fix or workaround, if ever.
    set(PNG_HARDWARE_OPTIMIZATIONS OFF)
    add_definitions(-DPNG_ARM_NEON_OPT=0)
endif()
set(PNG_TOOLS OFF)
set(PNG_TESTS OFF)
set(SKIP_INSTALL_EXPORT ON)
set(SKIP_INSTALL_CONFIG_FILE ON)

add_subdirectory("${AVITAB_png_SOURCE_DIR}" "${AVITAB_png_BINARY_DIR}" EXCLUDE_FROM_ALL)
add_library(png ALIAS png_static)

target_include_directories(png_static
    INTERFACE "$<BUILD_INTERFACE:${AVITAB_png_SOURCE_DIR}>"
    INTERFACE "$<BUILD_INTERFACE:${AVITAB_png_BINARY_DIR}>"
)

# This is to influence findPackage into detecting Avitab's version of PNG
set(PNG_PNG_INCLUDE_DIR "${AVITAB_png_SOURCE_DIR}" CACHE INTERNAL "")
if(MSVC)
    set(PNG_LIBRARY "${AVITAB_LIBS3RD_DIR}/png.lib" CACHE INTERNAL "")
else()
    set(PNG_LIBRARY "${AVITAB_LIBS3RD_DIR}/libpng.a" CACHE INTERNAL "")
endif()

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_png_PKGLIBS png CACHE INTERNAL "")
set(AVITAB_png_LOCATION "${PNG_LIBRARY}" CACHE INTERNAL "")

# This file is part of the Avitab project. See the README and LICENSE for details.

# The openjpeg downloaded library has CMake support, but the supported versions
# don't overlap with the minimum version Avitab requires. There may be quite a
# few warnings!

# Need to use cache variables due to policy CMP0077
set(BUILD_CODEC OFF CACHE INTERNAL "")

add_subdirectory("${AVITAB_openjpeg_SOURCE_DIR}" "${AVITAB_openjpeg_BINARY_DIR}" EXCLUDE_FROM_ALL)

target_include_directories(openjp2
    INTERFACE "$<BUILD_INTERFACE:${AVITAB_openjpeg_SOURCE_DIR}/src/lib/openjp2>"
    INTERFACE "$<BUILD_INTERFACE:${AVITAB_openjpeg_BINARY_DIR}/src/lib/openjp2>"
)

# Unset these after configuring openjpeg to avoid polluting other modules
unset(BUILD_CODEC CACHE)
unset(LIBRARY_OUTPUT_PATH CACHE)
unset(EXECUTABLE_OUTPUT_PATH CACHE)

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_openjpeg_PKGLIBS openjp2 CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_openjp2_LOCATION "${AVITAB_LIBS3RD_DIR}/openjp2.lib" CACHE INTERNAL "")
else()
    set(AVITAB_openjp2_LOCATION "${AVITAB_LIBS3RD_DIR}/libopenjp2.a" CACHE INTERNAL "")
endif()

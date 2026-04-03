# This file is part of the Avitab project. See the README and LICENSE for details.

set(HB_HAVE_FREETYPE ON)
set(HB_BUILD_SUBSET OFF)

add_subdirectory("${AVITAB_harfbuzz_SOURCE_DIR}" "${AVITAB_harfbuzz_BINARY_DIR}" EXCLUDE_FROM_ALL)

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_harfbuzz_PKGLIBS harfbuzz CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_harfbuzz_LOCATION "${AVITAB_LIBS3RD_DIR}/harfbuzz.lib" CACHE INTERNAL "")
else()
    set(AVITAB_harfbuzz_LOCATION "${AVITAB_LIBS3RD_DIR}/libharfbuzz.a" CACHE INTERNAL "")
endif()

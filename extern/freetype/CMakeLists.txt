# This file is part of the Avitab project. See the README and LICENSE for details.

# FreeType's CMake support is quite old, so cache variables are required to override options

# These HarfBuzz definitions are required so that Freetype finds the Avitab HB libraries.
# NB, hard-coded items will need updating if a new HarfBuzz package version is adopted.
set(HarfBuzz_INCLUDE_DIR "${AVITAB_DOWNLOAD_DIR}/harfbuzz-11.2.1/src" CACHE INTERNAL "")
if(MSVC)
    set(HarfBuzz_LIBRARY "${AVITAB_LIBS3RD_DIR}/harfbuzz.lib" CACHE INTERNAL "")
else()
    set(HarfBuzz_LIBRARY "${AVITAB_LIBS3RD_DIR}/libharfbuzz.a" CACHE INTERNAL "")
endif()

add_subdirectory("${AVITAB_freetype_SOURCE_DIR}" "${AVITAB_freetype_BINARY_DIR}" EXCLUDE_FROM_ALL)

target_link_libraries(freetype PRIVATE png bzip2)

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_freetype_PKGLIBS freetype CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_freetype_LOCATION "${AVITAB_LIBS3RD_DIR}/freetype.lib" CACHE INTERNAL "")
else()
    set(AVITAB_freetype_LOCATION "${AVITAB_LIBS3RD_DIR}/libfreetype.a" CACHE INTERNAL "")
endif()
set(AVITAB_freetype_LINKLIBS png bzip2 CACHE INTERNAL "")
set(AVITAB_freetype_INCDIRS "${AVITAB_freetype_SOURCE_DIR}/include" CACHE INTERNAL "")

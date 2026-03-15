# This file is part of the Avitab project. See the README and LICENSE for details.

# LunaSVG provides libraries for rendering SVG images to bitmaps.

add_subdirectory("${AVITAB_lunasvg_SOURCE_DIR}" "${AVITAB_lunasvg_BINARY_DIR}" EXCLUDE_FROM_ALL)

# Cache settings to be used when BUILD_AVITAB_THIRDPARTY is OFF
set(AVITAB_lunasvg_PKGLIBS lunasvg plutovg CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_lunasvg_LOCATION "${AVITAB_LIBS3RD_DIR}/lunasvg.lib" CACHE INTERNAL "")
    set(AVITAB_plutovg_LOCATION "${AVITAB_LIBS3RD_DIR}/plutovg.lib" CACHE INTERNAL "")
else()
    set(AVITAB_lunasvg_LOCATION "${AVITAB_LIBS3RD_DIR}/liblunasvg.a" CACHE INTERNAL "")
    set(AVITAB_plutovg_LOCATION "${AVITAB_LIBS3RD_DIR}/libplutovg.a" CACHE INTERNAL "")
endif()
set(AVITAB_lunasvg_CPPDEFS "LUNASVG_BUILD_STATIC" CACHE INTERNAL "")
set(AVITAB_lunasvg_INCDIRS "${AVITAB_lunasvg_SOURCE_DIR}/include" CACHE INTERNAL "")
set(AVITAB_lunasvg_LINKLIBS plutovg CACHE INTERNAL "")

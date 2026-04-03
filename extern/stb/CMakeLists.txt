# This file is part of the Avitab project. See the README and LICENSE for details.

# This file is a basic CMake wrapper for the stb external library, which does not
# provide CMake support natively.

project(stb
    VERSION 0.1.2
    DESCRIPTION "A library for various bits and pieces"
    LANGUAGES C
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Threads REQUIRED)
include(Prefixer)

# The stb library
set(stb_SOURCES
    stb_c_lexer.h
    stb_connected_components.h
    stb_divide.h
    stb_ds.h
    stb_dxt.h
    stb_easy_font.h
    stb_herringbone_wang_tile.h
    stb_hexwave.h
    stb_image_resize2.h
    stb_image_write.h
    stb_image.h
    stb_include.h
    stb_leakcheck.h
    stb_perlin.h
    stb_rect_pack.h
    stb_sprintf.h
    stb_textedit.h
    stb_tilemap_editor.h
    stb_truetype.h
    stb_voxel_render.h
)
PrefixSources(stb_SOURCES "${AVITAB_stb_SOURCE_DIR}")

add_library(stb STATIC ${stb_SOURCES} "${CMAKE_CURRENT_LIST_DIR}/phony.c")
target_include_directories(stb
    PUBLIC "${AVITAB_stb_SOURCE_DIR}"
)

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_stb_PKGLIBS stb CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_stb_LOCATION "${AVITAB_LIBS3RD_DIR}/stb.lib" CACHE INTERNAL "")
else()
    set(AVITAB_stb_LOCATION "${AVITAB_LIBS3RD_DIR}/libstb.a" CACHE INTERNAL "")
endif()
set(AVITAB_stb_INCDIRS "${AVITAB_stb_SOURCE_DIR}" CACHE INTERNAL "")

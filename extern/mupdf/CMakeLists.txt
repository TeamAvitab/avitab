# This file is part of the Avitab project. See the README and LICENSE for details.

# Avitab custom CMake for the MuPDF library, since it doesn't provide one.

project(MuPDF
    VERSION 1.26.3
    DESCRIPTION "A library for rendering PDF and other formats"
    LANGUAGES C
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Threads REQUIRED)
include(Prefixer)

file(GLOB MUPDF_SOURCES
    "${AVITAB_mupdf_SOURCE_DIR}/source/fitz/*.c"
    "${AVITAB_mupdf_SOURCE_DIR}/source/html/*.c"
    "${AVITAB_mupdf_SOURCE_DIR}/source/pdf/*.c"
    "${AVITAB_mupdf_SOURCE_DIR}/source/svg/*.c"
    "${AVITAB_mupdf_SOURCE_DIR}/generated/resources/fonts/urw/*.cff.c"
    "${AVITAB_mupdf_SOURCE_DIR}/source/cbz/muimg.c"
)

add_library(mupdf STATIC ${MUPDF_SOURCES})
target_include_directories(mupdf
    PUBLIC "${AVITAB_mupdf_SOURCE_DIR}/include"
)
target_link_libraries(mupdf
    PRIVATE zlibs
    PRIVATE brotlidec
    PRIVATE brotlienc
    PRIVATE lcms2
    PRIVATE jbig2dec
    PRIVATE jpeg
    PRIVATE openjp2
    PRIVATE freetype
    PRIVATE harfbuzz
    PRIVATE gumbo
)

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_mupdf_PKGLIBS mupdf CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_mupdf_LOCATION "${AVITAB_LIBS3RD_DIR}/mupdf.lib" CACHE INTERNAL "")
else()
    set(AVITAB_mupdf_LOCATION "${AVITAB_LIBS3RD_DIR}/libmupdf.a" CACHE INTERNAL "")
endif()
set(AVITAB_mupdf_INCDIRS "${AVITAB_mupdf_SOURCE_DIR}/include" CACHE INTERNAL "")
set(AVITAB_mupdf_LINKLIBS zlibs brotlidec brotlienc brotlicommon lcms2 jbig2dec jpeg openjp2 freetype harfbuzz gumbo CACHE INTERNAL "")

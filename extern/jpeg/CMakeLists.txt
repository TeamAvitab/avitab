# This file is part of the Avitab project. See the README and LICENSE for details.

# The jpeg downloaded library has CMake support, but it is out of date and
# produces multiple warnings. So Avitab uses its own configuration with some
# chunks of the original.

set(BUILD_EXECUTABLES OFF)
set(BUILD_TESTS  OFF)
set(LINK_STATIC  OFF)
set(BUILD_ALT_UI  OFF)

project(LibJPEG
    VERSION 1.9.6
    DESCRIPTION "The independent JPEG group library"
    LANGUAGES C
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Threads REQUIRED)
include(Prefixer)

set(LIBJPEG_SOURCES
    jaricom.c jcapimin.c jcapistd.c jcarith.c jccoefct.c jccolor.c
    jcdctmgr.c jchuff.c jcinit.c jcmainct.c jcmarker.c jcmaster.c
    jcomapi.c jcparam.c jcprepct.c jcsample.c jctrans.c jdapimin.c
    jdapistd.c jdarith.c jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c
    jddctmgr.c jdhuff.c jdinput.c jdmainct.c jdmarker.c jdmaster.c
    jdmerge.c jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c
    jfdctfst.c jfdctint.c jidctflt.c jidctfst.c jidctint.c jquant1.c
    jquant2.c jutils.c jmemmgr.c
)
PrefixSources(LIBJPEG_SOURCES "${AVITAB_jpeg_SOURCE_DIR}")

file(COPY_FILE "${AVITAB_jpeg_SOURCE_DIR}/jconfig.txt" "${CMAKE_CURRENT_BINARY_DIR}/jconfig.h")

add_library(jpeg STATIC ${LIBJPEG_SOURCES})
target_include_directories(jpeg
    PUBLIC "${CMAKE_CURRENT_BINARY_DIR}"
    PUBLIC "${AVITAB_jpeg_SOURCE_DIR}"
)

# this aims to help find_package select this jpeg library to help out 
add_library(JPEG::JPEG ALIAS jpeg)
set(JPEG_INCLUDE_DIR "${AVITAB_jpeg_SOURCE_DIR}" CACHE INTERNAL "")
if(MSVC)
    set(JPEG_LIBRARY "${AVITAB_LIBS3RD_DIR}/jpeg.lib" CACHE INTERNAL "")
else()
    set(JPEG_LIBRARY "${AVITAB_LIBS3RD_DIR}/libjpeg.a" CACHE INTERNAL "")
endif()

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_jpeg_PKGLIBS jpeg CACHE INTERNAL "")
set(AVITAB_jpeg_LOCATION "${JPEG_LIBRARY}" CACHE INTERNAL "")

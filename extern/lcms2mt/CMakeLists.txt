# This file is part of the Avitab project. See the README and LICENSE for details.

# Project name and a few useful settings. Other commands can pick up the results
project(LCMS2MT
    VERSION 2.2.17
    DESCRIPTION "A library for color management, with MT support from Artifex."
    LANGUAGES C
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Threads REQUIRED)
include(Prefixer)

# The LCMS2MT library
set(LCMS2MT_SOURCES
    cmsalpha.c
    cmscam02.c
    cmscgats.c
    cmscnvrt.c
    cmserr.c
    cmsgamma.c
    cmsgmt.c
    cmshalf.c
    cmsintrp.c
    cmsio0.c
    cmsio1.c
    cmslut.c
    cmsmd5.c
    cmsmtrx.c
    cmsnamed.c
    cmsopt.c
    cmspack.c
    cmspcs.c
    cmsplugin.c
    cmsps2.c
    cmssamp.c
    cmssm.c
    cmstypes.c
    cmsvirt.c
    cmswtpnt.c
    cmsxform.c
)
PrefixSources(LCMS2MT_SOURCES "${AVITAB_lcms2mt_SOURCE_DIR}/src")

add_library(lcms2mt STATIC ${LCMS2MT_SOURCES})
target_compile_definitions(lcms2mt
    PUBLIC LCMS2MT_PREFIX=lcms2mt_
    INTERFACE HAVE_LCMS2MT
)
target_include_directories(lcms2mt
    PUBLIC "${AVITAB_lcms2mt_SOURCE_DIR}/include"
)

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_lcms2mt_PKGLIBS lcms2mt CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_lcms2mt_LOCATION "${AVITAB_LIBS3RD_DIR}/lcms2mt.lib" CACHE INTERNAL "")
else()
    set(AVITAB_lcms2mt_LOCATION "${AVITAB_LIBS3RD_DIR}/liblcms2mt.a" CACHE INTERNAL "")
endif()

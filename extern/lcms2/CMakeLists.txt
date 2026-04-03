# This file is part of the Avitab project. See the README and LICENSE for details.

# Project name and a few useful settings. Other commands can pick up the results
project(LCMS2
    VERSION 2.2.17
    DESCRIPTION "A library for color management"
    LANGUAGES C
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Threads REQUIRED)
include(Prefixer)

# The LCMS2 library
set(LCMS2_SOURCES
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
PrefixSources(LCMS2_SOURCES "${AVITAB_lcms2_SOURCE_DIR}/src")

add_library(lcms2 STATIC ${LCMS2_SOURCES})
target_include_directories(lcms2
    PUBLIC "${AVITAB_lcms2_SOURCE_DIR}/include"
)

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_lcms2_PKGLIBS lcms2 CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_lcms2_LOCATION "${AVITAB_LIBS3RD_DIR}/lcms2.lib" CACHE INTERNAL "")
else()
    set(AVITAB_lcms2_LOCATION "${AVITAB_LIBS3RD_DIR}/liblcms2.a" CACHE INTERNAL "")
endif()

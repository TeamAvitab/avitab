# This file is part of the Avitab project. See the README and LICENSE for details.

# This file stubs the wrapper for the X-Plane SDK. A static library is built
# to satisfy the requirements of the ImportExport module, but it is not linked
# into the Avitab products.

project(xsdk
    VERSION 4.3.0
    DESCRIPTION "Phony library target for the X-Plane SDK"
    LANGUAGES C
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Threads REQUIRED)
include(Prefixer)
add_library(xsdk STATIC ${xsdk_SOURCES} "${CMAKE_CURRENT_LIST_DIR}/phony.c")

# Cache settings to be used when BUILD_AVITAB_THIRDPARTY is OFF
set(AVITAB_xsdk_PKGLIBS xsdk CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_xsdk_LOCATION "${AVITAB_LIBS3RD_DIR}/xsdk.lib" CACHE INTERNAL "")
else()
    set(AVITAB_xsdk_LOCATION "${AVITAB_LIBS3RD_DIR}/libxsdk.a" CACHE INTERNAL "")
endif()
set(XPLANE_SDK "${AVITAB_xsdk_SOURCE_DIR}" CACHE INTERNAL "")

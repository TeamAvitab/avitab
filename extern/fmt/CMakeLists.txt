# This file is part of the Avitab project. See the README and LICENSE for details.

# Formatting library, adds fmt::fmt to C++ namespace

set(FMT_INSTALL OFF)

add_subdirectory("${AVITAB_fmt_SOURCE_DIR}" "${AVITAB_fmt_BINARY_DIR}" EXCLUDE_FROM_ALL)

# Cache settings to be used when BUILD_AVITAB_THIRDPARTY is OFF
set(AVITAB_fmt_PKGLIBS fmt CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_fmt_LOCATION "${AVITAB_LIBS3RD_DIR}/fmt.lib" CACHE INTERNAL "")
else()
    set(AVITAB_fmt_LOCATION "${AVITAB_LIBS3RD_DIR}/libfmt.a" CACHE INTERNAL "")
endif()
set(AVITAB_fmt_INCDIRS "${AVITAB_fmt_SOURCE_DIR}/include" CACHE INTERNAL "")

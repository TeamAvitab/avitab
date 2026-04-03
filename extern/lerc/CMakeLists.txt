# This file is part of the Avitab project. See the README and LICENSE for details.

add_subdirectory("${AVITAB_lerc_SOURCE_DIR}" "${AVITAB_lerc_BINARY_DIR}" EXCLUDE_FROM_ALL)
add_library(lerc ALIAS Lerc)

# This is to influence findPackage into detecting Avitab's version of Lerc
set(LERC_INCLUDE_DIR "${AVITAB_lerc_SOURCE_DIR}/src/LercLib/include" CACHE INTERNAL "")
if(MSVC)
    set(LERC_LIBRARY "${AVITAB_LIBS3RD_DIR}/lerc.lib" CACHE INTERNAL "")
else()
    set(LERC_LIBRARY "${AVITAB_LIBS3RD_DIR}/liblerc.a" CACHE INTERNAL "")
endif()

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_lerc_PKGLIBS lerc CACHE INTERNAL "")
set(AVITAB_lerc_LOCATION "${LERC_LIBRARY}" CACHE INTERNAL "")

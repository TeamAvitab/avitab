# This file is part of the Avitab project. See the README and LICENSE for details.

# This file is a basic CMake wrapper for the geotiff external library, used instead
# of the provided CMake support.

set(SAVED_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(GEOTIFF_CSV_DATA_DIR "${AVITAB_geotiff_SOURCE_DIR}/libgeotiff/csv")
set(WITH_UTILITIES FALSE CACHE BOOL "Choose if GeoTIFF utilities should be built")
#unset(TIFF_FOUND CACHE)
set(HAVE_TIFFOPEN TRUE)
set(HAVE_TIFFMERGEFIELDINFO TRUE)

add_subdirectory("${AVITAB_geotiff_SOURCE_DIR}/libgeotiff" "${AVITAB_geotiff_BINARY_DIR}" EXCLUDE_FROM_ALL)

add_library(geotiff ALIAS geotiff_library)

#unset(TIFF_FOUND CACHE)
set(CMAKE_MODULE_PATH ${SAVED_CMAKE_MODULE_PATH})
unset(SAVED_CMAKE_MODULE_PATH)

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_geotiff_PKGLIBS geotiff CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_geotiff_LOCATION "${AVITAB_LIBS3RD_DIR}/geotiff.lib" CACHE INTERNAL "")
else()
    set(AVITAB_geotiff_LOCATION "${AVITAB_LIBS3RD_DIR}/libgeotiff.a" CACHE INTERNAL "")
endif()
set(AVITAB_geotiff_INCDIRS "${AVITAB_geotiff_BINARY_DIR}" "${AVITAB_geotiff_SOURCE_DIR}/libgeotiff" ${AVITAB_tiff_INCDIRS} CACHE INTERNAL "")
set(AVITAB_geotiff_LINKLIBS tiff proj CACHE INTERNAL "")

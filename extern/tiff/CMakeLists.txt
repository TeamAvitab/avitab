# This file is part of the Avitab project. See the README and LICENSE for details.

# tiff has cmake support which is quite old and generates warnings, and quite
# a lot of noise. for simplicity we've disabled support for various codecs that
# we can't be bothered to add speculatively. potentially there might be a need for
# these that will be discovered one day. at which point the problem can be resolved.

set(tiff-tools OFF CACHE INTERNAL "")
set(tiff-tests OFF CACHE INTERNAL "")
set(tiff-contrib OFF CACHE INTERNAL "")
set(tiff-docs OFF CACHE INTERNAL "")
set(libdeflate OFF CACHE INTERNAL "")
set(pixarlog OFF CACHE INTERNAL "")
set(jbig OFF CACHE INTERNAL "")
set(lzma OFF CACHE INTERNAL "")
set(webp OFF CACHE INTERNAL "")
set(ZSTD_HAVE_DECOMPRESS_STREAM ON)

add_subdirectory("${AVITAB_tiff_SOURCE_DIR}" "${AVITAB_tiff_BINARY_DIR}" EXCLUDE_FROM_ALL)

target_link_libraries(tiff INTERFACE lerc jpeg zlibs zstd)

unset(tiff-tools CACHE)
unset(tiff-tests CACHE)
unset(tiff-contrib CACHE)
unset(tiff-docs CACHE)
unset(libdeflate CACHE)
unset(pixarlog CACHE)
unset(jbig CACHE)
unset(lzma CACHE)
unset(webp CACHE)


# This is to influence findPackage into detecting Avitab's version of Tiff
set(TIFF_INCLUDE_DIR "${AVITAB_tiff_SOURCE_DIR}/libtiff" CACHE INTERNAL "")
if(MSVC)
    set(TIFF_LIBRARY "${AVITAB_LIBS3RD_DIR}/tiff.lib" CACHE INTERNAL "")
else()
    set(TIFF_LIBRARY "${AVITAB_LIBS3RD_DIR}/libtiff.a" CACHE INTERNAL "")
endif()

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_tiff_PKGLIBS tiff CACHE INTERNAL "")
set(AVITAB_tiff_LOCATION ${TIFF_LIBRARY} CACHE INTERNAL "")
set(AVITAB_tiff_INCDIRS "${AVITAB_tiff_BINARY_DIR}/libtiff" "${AVITAB_tiff_SOURCE_DIR}/libtiff" "${AVITAB_tiff_SOURCE_DIR}/contrib" CACHE INTERNAL "")
set(AVITAB_tiff_LINKLIBS lerc jpeg zlibs zstd CACHE INTERNAL "")

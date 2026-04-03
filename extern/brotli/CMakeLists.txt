# This file is part of the Avitab project. See the README and LICENSE for details.

set(BROTLI_DISABLE_TESTS ON)

add_subdirectory("${AVITAB_brotli_SOURCE_DIR}" "${AVITAB_brotli_BINARY_DIR}" EXCLUDE_FROM_ALL)

# This is to influence various uses of findPackage into detecting Avitab's version of BrotliDec
set(BROTLI_INCLUDE_DIR "${AVITAB_brotli_SOURCE_DIR}/c/include" CACHE INTERNAL "")
set(BROTLIDEC_INCLUDE_DIRS "${AVITAB_brotli_SOURCE_DIR}/c/include" CACHE INTERNAL "")
if(MSVC)
    set(BROTLICOMMON_LIBRARY "${AVITAB_LIBS3RD_DIR}/brotlicommon.lib" CACHE INTERNAL "")
    set(BROTLIDEC_LIBRARY "${AVITAB_LIBS3RD_DIR}/brotlidec.lib" CACHE INTERNAL "")
    set(BROTLIENC_LIBRARY "${AVITAB_LIBS3RD_DIR}/brotlienc.lib")
else()
    set(BROTLICOMMON_LIBRARY "${AVITAB_LIBS3RD_DIR}/libbrotlicommon.a" CACHE INTERNAL "")
    set(BROTLIDEC_LIBRARY "${AVITAB_LIBS3RD_DIR}/libbrotlidec.a" CACHE INTERNAL "")
    set(BROTLIENC_LIBRARY "${AVITAB_LIBS3RD_DIR}/libbrotlienc.a")
endif()
set(BROTLIDEC_LIBRARIES ${BROTLICOMMON_LIBRARY} ${BROTLIDEC_LIBRARY} ${BROTLIENC_LIBRARY} CACHE INTERNAL "")

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_brotli_PKGLIBS brotlidec brotlienc brotlicommon CACHE INTERNAL "")
set(AVITAB_brotlidec_LOCATION "${BROTLIDEC_LIBRARY}" CACHE INTERNAL "")
set(AVITAB_brotlienc_LOCATION "${BROTLIENC_LIBRARY}" CACHE INTERNAL "")
set(AVITAB_brotlicommon_LOCATION "${BROTLICOMMON_LIBRARY}" CACHE INTERNAL "")

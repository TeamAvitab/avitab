# This file is part of the Avitab project. See the README and LICENSE for details.

# This script handles all of the 3rd party libraries that are used by the
# Avitab project. A number of these projects do not support CMake natively,
# and in those cases a rudimentary CMake wrapper has been created and is
# patched into the source tree after downloading. The goal is to have a
# fully cross-platform build system (specifically supporting VS/MSVC).

message(STATUS "Checking if third-party libraries need to be built")

include(ExternPackages)

# These packages are ordered so that their cmake configuration dependencies can be satisfied
# within the project. We want to avoid discovering and linking against a shared library
# on the development system that might not be present on the user's system, and we need
# to guarantee to link the X-Plane plugin statically (X-Plane requirement).

PrepareExternPackages(
    # First group have no dependencies, so order (within the group) is not important
    xsdk        "https://github.com/TeamAvitab/XSDK/archive/refs/tags/v4.3.0.tar.gz"
    fmt         "https://github.com/fmtlib/fmt/releases/download/10.2.1/fmt-10.2.1.zip"
    json        "https://github.com/nlohmann/json/archive/refs/tags/v3.12.0.tar.gz"
    mbedtls     "https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-3.6.4/mbedtls-3.6.4.tar.bz2"
    detex       "https://github.com/hglm/detex/archive/refs/tags/v0.1.2alpha2.tar.gz"
    stb         "https://github.com/nothings/stb/archive/449758b.tar.gz"
    sqlite      "https://www.sqlite.org/2025/sqlite-amalgamation-3500300.zip"
    lunasvg     "https://github.com/sammycage/lunasvg/archive/refs/tags/v3.4.0.zip"
    zlib        "https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz"
    bzip2       "https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz"
    brotli      "https://github.com/google/brotli/archive/refs/tags/v1.1.0.tar.gz"
    lerc        "https://github.com/Esri/lerc/archive/refs/tags/v4.0.0.tar.gz"
    zstd        "https://github.com/facebook/zstd/archive/refs/tags/v1.5.7.tar.gz"
    gumbo       "https://github.com/google/gumbo-parser/archive/refs/tags/v0.10.1.tar.gz"
    lcms2mt     "https://github.com/ArtifexSoftware/thirdparty-lcms2/archive/f75fad7.zip"
    jpeg        "https://www.ijg.org/files/jpegsrc.v9f.tar.gz"
    openjpeg    "https://github.com/uclouvain/openjpeg/archive/refs/tags/v2.5.3.tar.gz"
    glfw        "https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz"
    lvgl        "https://github.com/lvgl/lvgl/archive/refs/tags/v6.1.2.tar.gz"

    # Remaining packages have dependencies, so order matters.
    # jbig2 appears to use zlib
    jbig2       "https://github.com/ArtifexSoftware/jbig2dec/archive/refs/tags/0.20.tar.gz"
    # proj uses sqlite (apparently?).
    proj        "https://github.com/OSGeo/proj.4/archive/refs/tags/5.2.0.tar.gz"
    # png requires zlib
    png         "https://github.com/pnggroup/libpng/archive/refs/tags/v1.6.50.tar.gz"
    # tiff uses zlib, jpeg, lerc, zstd
    # (tiff could use deflate, jbig, liblzma, wbep, but these are not currently provided or configured)
    tiff        "https://gitlab.com/libtiff/libtiff/-/archive/v4.7.0/libtiff-v4.7.0.tar.gz"
    # geotiff uses tiff, proj.
    geotiff     "https://github.com/OSGeo/libgeotiff/archive/refs/tags/1.4.3.tar.gz"
    # ssh2 is configured to use mbedtls
    ssh2        "https://github.com/libssh2/libssh2/releases/download/libssh2-1.11.1/libssh2-1.11.1.tar.gz"
    # curl uses zlib, zstd, brotli, ssh2, mbedtls
    curl        "https://curl.se/download/curl-8.15.0.tar.gz"
    # freetype uses zlib, bzip2, brotli, png, and harfbuzz (circular-dependency).
    freetype    "https://download.savannah.gnu.org/releases/freetype/freetype-2.13.3.tar.gz"
    # harfbuzz uses freetype, but will use Avitab's freetype if it is a build target, so is processed afterwards
    harfbuzz    "https://github.com/harfbuzz/harfbuzz/archive/refs/tags/11.2.1.tar.gz"
    # mupdf uses zlib, brotli, lcms2mt, jbig2, jpeg, openjpeg, freetype, harfbuzz, gumbo
    mupdf       "https://github.com/ArtifexSoftware/mupdf/archive/refs/tags/1.26.12.tar.gz"
)

# We don't want any of the packages to build shared libs, examples or testing.
# These variables are common to more than one external library, so we set them once here.
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(SKIP_INSTALL_ALL ON CACHE INTERNAL "")
set(BUILD_TESTING_saved ${BUILD_TESTING}) # keep for later restore
set(BUILD_TESTING OFF CACHE BOOL "Include tests in build" FORCE)

# Configure the external packages. If any of them need building then they will all
# be built, this is mainly because the alternative is not worth the extra effort!
unset(AVITAB_ALL_PKGLIBS CACHE) # obsolete cache variable from earlier versions
if(AVITAB_OUT_OF_DATE_PACKAGES)
    # Slightly paranoid, but to make sure that there is at least one full build
    # after this configuration, remove the external libraries 'install' directory.
    # Future configurations will not be optimised until after at least one full build.
    file(REMOVE_RECURSE "${AVITAB_LIBS3RD_DIR}")
    file(MAKE_DIRECTORY "${AVITAB_LIBS3RD_DIR}")
endif()
if(AVITAB_FORCE_BUILD_THIRDPARTY OR AVITAB_OUT_OF_DATE_PACKAGES)
    ConfigureExternPackages(${AVITAB_ALL_PACKAGES})
endif()

# Restore any cache variables that Avitab makes use of itself.
set(BUILD_TESTING ${BUILD_TESTING_saved} CACHE BOOL "Include tests in build" FORCE)
unset(BUILD_TESTING_saved)

# Collect together a list of all the library targets - some packages create more than one.
set(AVITAB_ALL_PKGLIBS)
foreach(name ${AVITAB_ALL_PACKAGES})
    set(AVITAB_ALL_PKGLIBS ${AVITAB_ALL_PKGLIBS} ${AVITAB_${name}_PKGLIBS})
endforeach()

# Create a proxy library that is used as a simple dependency target by the avitab core
# to ensure all 3rd party libraries are built before linking.
add_library(avitab_extern INTERFACE)

# If the external libraries will be built then install them to libs3rd and
# make them a dependency of avitab_extern.
if(AVITAB_FORCE_BUILD_THIRDPARTY OR AVITAB_OUT_OF_DATE_PACKAGES)
    InstallExternLibraries(${AVITAB_ALL_PKGLIBS})
    add_dependencies(avitab_extern ${AVITAB_ALL_INSTALLED_LIBS})
endif()

# Import (declare proxy (interface) libraries for) each third-party library.
message(STATUS "Importing third-party libraries")
ImportExternLibraries(${AVITAB_ALL_PKGLIBS})

# General configuration when linking with the 3rd-party libraries
if(WIN32)
    set(sys_libraries
        iphlpapi advapi32 crypt32 ws2_32 bcrypt
    )
elseif(APPLE)
    set(sys_libraries
        "-lm" "-framework SystemConfiguration" "-framework CoreFoundation"
        "-framework CoreServices" "-framework ApplicationServices"
        "-framework Cocoa" "-framework IOKit"
    )
endif()

target_link_libraries(avitab_extern INTERFACE
    Threads::Threads
    nlohmann_json_imp curl_imp mupdf_imp lvgl_imp sqlite3_imp
    freetype_imp mbedtls_imp detex_imp stb_imp geotiff_imp zstd_imp
    ${sys_libraries}
)

# hack to resolve bare library names appearing in the link (root cause unresolved)
target_link_directories(avitab_extern INTERFACE
    ${AVITAB_LIBS3RD_DIR}
)

# This file is part of the Avitab project. See the README and LICENSE for details.

# This script handles all of the 3rd party libraries that are used by the
# Avitab project. A number of these projects do not support CMake natively,
# and in those cases a rudimentary CMake wrapper has been created and is
# patched into the source tree after downloading. The goal is to have a
# fully cross-platform build system (specifically supporting VS/MSVC).

include(ImportExport)

# Determine whether or not we can generate an optimised build
if(NOT AVITAB_ALL_PKGLIBS)
    set(full_build ON)
else()
    set(full_build OFF)
    # check each 3rd-party library exists
    foreach(lib ${AVITAB_ALL_PKGLIBS})
        if(NOT "${AVITAB_${lib}_LOCATION}" STREQUAL "")
            if(NOT EXISTS "${AVITAB_${lib}_LOCATION}")
                set(full_build ON)
            endif()
        endif()
    endforeach()
endif()

# Skip this section if we decided that all 3rd party libraries are downloaded, unpacked and pre-built
if(BUILD_AVITAB_THIRDPARTY OR full_build)
    message(STATUS "Downloading and configuring third-party sources")

    # These packages are ordered so that their cmake configuration dependencies can be satisfied
    # within the project. We want to avoid discovering and linking against a shared library
    # on the development system that might not be present on the user's system, and we need
    # to guarantee to link the X-Plane plugin statically (X-Plane requirement).

    set(PKGS
        # First group have no dependencies, so order is not so important

        xsdk        "https://github.com/TeamAvitab/XSDK/archive/refs/tags/v4.3.0.tar.gz"
        # xsdk has no dependencies.
        fmt         "https://github.com/fmtlib/fmt/releases/download/10.2.1/fmt-10.2.1.zip"
        # fmt has no dependencies.
        json        "https://github.com/nlohmann/json/archive/refs/tags/v3.12.0.tar.gz"
        # json has no dependencies.
        mbedtls     "https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-3.6.4/mbedtls-3.6.4.tar.bz2"
        # mbedtls
        detex       "https://github.com/hglm/detex/archive/refs/tags/v0.1.2alpha2.tar.gz"
        # detex has no dependencies.
        stb         "https://github.com/nothings/stb/archive/449758b.tar.gz"
        # stb has no dependencies.
        sqlite      "https://www.sqlite.org/2025/sqlite-amalgamation-3500300.zip"
        # sqlite has no dependencies. it does not have inbuilt CMake support.
        lunasvg     "https://github.com/sammycage/lunasvg/archive/refs/tags/v3.4.0.zip"
        # lunasvg has no dependencies.
        zlib        "https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz"
        # zlib has no dependencies.
        bzip2       "https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz"
        # bzip2 has no dependencies. it does not have inbuilt CMake support.
        brotli      "https://github.com/google/brotli/archive/refs/tags/v1.1.0.tar.gz"
        # brotli has no dependencies.
        jbig2       "https://github.com/ArtifexSoftware/jbig2dec/archive/refs/tags/0.20.tar.gz"
        # jbig2 appears to use zlib, and does not have inbuilt CMake support.
        lerc        "https://github.com/Esri/lerc/archive/refs/tags/v4.0.0.tar.gz"
        # lerc has no dependencies.
        zstd        "https://github.com/facebook/zstd/archive/refs/tags/v1.5.7.tar.gz"
        # ztsd has no dependencies.
        gumbo       "https://github.com/google/gumbo-parser/archive/refs/tags/v0.10.1.tar.gz"
        # gumbo has no dependencies. it does not have inbuilt CMake support.
        lcms2       "https://github.com/mm2/Little-CMS/releases/download/lcms2.17/lcms2-2.17.tar.gz"
        # lcms2 has no dependencies. it does not have inbuilt CMake support.
        jpeg        "https://www.ijg.org/files/jpegsrc.v9f.tar.gz"
        # jpeg has no dependencies. it does not have inbuilt CMake support.
        openjpeg    "https://github.com/uclouvain/openjpeg/archive/refs/tags/v2.5.3.tar.gz"
        # openjpeg has no dependencies.
        glfw        "https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz"
        # glfw has no dependencies.
        lvgl        "https://github.com/lvgl/lvgl/archive/refs/tags/v6.1.2.tar.gz"
        # lvgl has no dependencies.

        # Remaining packages have dependencies, so order matters.

        proj        "https://github.com/OSGeo/proj.4/archive/refs/tags/5.2.0.tar.gz"
        # proj uses sqlite (apparently?).
        png         "https://github.com/pnggroup/libpng/archive/refs/tags/v1.6.50.tar.gz"
        # png requires zlib
        tiff        "https://gitlab.com/libtiff/libtiff/-/archive/v4.7.0/libtiff-v4.7.0.tar.gz"
        # tiff uses zlib, jpeg, lerc, zstd
        # (tiff does not use deflate, jbig, liblzma, wbep, which are not currently provided or configured)
        geotiff     "https://github.com/OSGeo/libgeotiff/archive/refs/tags/1.4.3.tar.gz"
        # geotiff uses tiff, proj.
        ssh2        "https://github.com/libssh2/libssh2/releases/download/libssh2-1.11.1/libssh2-1.11.1.tar.gz"
        # ssh2 is configured to use mbedtls
        curl        "https://curl.se/download/curl-8.15.0.tar.gz"
        # curl uses zlib, zstd, brotli, ssh2, mbedtls
        harfbuzz_NOCONF    "https://github.com/harfbuzz/harfbuzz/archive/refs/tags/11.2.1.tar.gz"
        # download and unpack harfbuzz, but do not configure yet, allows freetype access to headers
        freetype    "https://download.savannah.gnu.org/releases/freetype/freetype-2.13.3.tar.gz"
        # freetype uses zlib, bzip2, brotli, png, and harfbuzz (circular-dependency).
        harfbuzz    "https://github.com/harfbuzz/harfbuzz/archive/refs/tags/11.2.1.tar.gz"
        # harfbuzz uses freetype, but will use Avitab's freetype if it is a build target, so is processed afterwards
        mupdf       "https://github.com/ArtifexSoftware/mupdf/archive/refs/tags/1.26.3.tar.gz"
        # mupdf uses zlib, brotli, lcms2, jbig2, jpeg, openjpeg, freetype, harfbuzz, gumbo
    )

    # We don't want any of the packages to build shared libs, examples or testing.
    # These variables are common to more than one library, so we set them once here.
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
    set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(SKIP_INSTALL_ALL ON CACHE INTERNAL "")
    set(BUILD_TESTING_saved ${BUILD_TESTING})
    set(BUILD_TESTING OFF CACHE BOOL "Include tests in build" FORCE)

    # Download and configure each of the third-party packages in turn
    set(libs_thirdparty)
    include(Downloader)
    foreach(x ${PKGS})
        if("${pkg}" STREQUAL "")
            set(pkg ${x})
        else()
            IncludeDownloadedPackage(${pkg} "${x}" libs_thirdparty)
            unset(pkg)
        endif()
    endforeach()
    set(AVITAB_ALL_PKGLIBS ${libs_thirdparty} CACHE INTERNAL "")

    set(copies)
    foreach(lib ${libs_thirdparty})
        set(libpath)
        ExportThirdPartyLibrary(${lib} libpath)
        set(copies ${copies} ${libpath})
    endforeach()
    add_dependencies(avitab_extern ${copies})

    # Restore any cache variables that Avitab makes use of itself.
    set(BUILD_TESTING ${BUILD_TESTING_saved} CACHE BOOL "Include tests in build" FORCE)
    unset(BUILD_TESTING_saved)

    message(STATUS "Finished making external content available")
endif(BUILD_AVITAB_THIRDPARTY OR full_build)

message(STATUS "Importing third-party libraries")
foreach(lib ${AVITAB_ALL_PKGLIBS})
    ImportThirdPartyLibrary("${lib}")
endforeach()
# declare the third party library interdependencies only after they have all been imported
foreach(lib ${AVITAB_ALL_PKGLIBS})
    unset(prevlib)
    foreach(tll ${AVITAB_${lib}_LINKLIBS})
        if(${tll} IN_LIST AVITAB_ALL_PKGLIBS)
            target_link_libraries(${lib}_imp INTERFACE ${tll}_imp ${prevlib})
        else()
            target_link_libraries(${lib}_imp INTERFACE ${tll})
        endif()
        set(prevlib "${tll}_imp") # this is a small hack to force link order and ensure depndencies are met
    endforeach()
endforeach()

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
    nlohmann_json_imp curl_imp mupdf_imp lvgl_imp sqlite3_imp glfw3_imp
    freetype_imp mbedtls_imp detex_imp stb_imp geotiff_imp zstd_imp
    ${sys_libraries}
)

# hack to resolve bare library names appearing in the link (root cause unresolved)
target_link_directories(avitab_extern INTERFACE
    ${AVITAB_LIBS3RD_DIR}
)

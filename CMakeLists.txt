# This file is part of the Avitab project. See the README and LICENSE for details.

# Works with 3.23 and tested through 3.29
cmake_minimum_required(VERSION 3.23...3.29)

# Stuff that might affect the way the project command works
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE INTERNAL "Minimum macOS version" FORCE)
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "Build architectures for mac OS X" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Avitab requires static libraries")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE INTERNAL "Link MSVC static runtime, required by X-Plane plugin")
set(CMAKE_FRAMEWORK FALSE CACHE INTERNAL "")

# Project name and a few useful settings. Other commands can pick up the results
project(Avitab
    VERSION 0.8.0
    DESCRIPTION "AviTab add-on for X-Plane and Microsoft Flight Simulator"
    LANGUAGES C CXX
)

# This project is not currently designed to be imported into another project.
# Maybe a later update will split the libraries into separate repos??
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(FATAL_ERROR "Avitab is not supported as a subsidiary CMake project")
endif()

# Various CMake configuration bits and pieces
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Nicely support folders in IDEs
find_package(Threads)

# Project options
option(BUILD_AVITAB_THIRDPARTY
    "If set then the 3rd-party libraries will always be built. If unset the libraries will only be built if they are missing." OFF
)

set(download_dir "${PROJECT_BINARY_DIR}/../download")
cmake_path(NORMAL_PATH download_dir)
set(AVITAB_DOWNLOAD_DIR "${download_dir}" CACHE PATH "Directory for 3rd-party package downloads. Can be shared between build configurations.")
file(MAKE_DIRECTORY "${AVITAB_DOWNLOAD_DIR}")

string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
set(libs_3rdparty_dir "${PROJECT_BINARY_DIR}/../libs3rd/${build_type}")
cmake_path(NORMAL_PATH libs_3rdparty_dir)
set(AVITAB_LIBS3RD_DIR "${libs_3rdparty_dir}" CACHE PATH "Directory for installing 3rd-party libraries after full build.")
file(MAKE_DIRECTORY "${AVITAB_LIBS3RD_DIR}")

# Set things like CMAKE_CXX_STANDARD here
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF) # ensure -std=c++xx instead of -std=g++xx
if(WIN32 AND MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.1")
endif()

# Testing
include(CTest)

# Docs
add_custom_target(avitab_docs SOURCES README.md)
find_package(Doxygen)
if(NOT Doxygen_FOUND)
    message(STATUS "Doxygen not found, not building docs")
endif()
add_subdirectory(docs)

# External (third-party) source packages are downloaded and configured here.
# Create a single interface library for Avitab which references the packages required.
add_library(avitab_extern INTERFACE)
add_subdirectory(extern)

# Check for MSFS SDK and set variables if defined
if(WIN32)
    if(DEFINED ENV{MSFS_SDK} AND EXISTS "$ENV{MSFS_SDK}/SimConnect SDK/include/SimConnect.h")
        cmake_path(SET MSFS_SDK NORMALIZE "$ENV{MSFS_SDK}")
        message(STATUS "Detected Microsoft Flight Simulator SDK @ ${MSFS_SDK}")
    else()
        message(STATUS "The MSFS SDK was not defined by env(MSFS_SDK). MSFS add-ons will not be built.")
    endif()
endif()

# Check for Navigraph and Chartfox API keys
if(DEFINED ENV{AVITAB_APIKEY_NAVIGRAPH})
    set(NAVIGRAPH_CLIENT_SECRET $ENV{AVITAB_APIKEY_NAVIGRAPH})
    message(STATUS "Navigraph API key has been defined.")
endif()
if(DEFINED ENV{AVITAB_APIKEY_CHARTFOX})
    set(CHARTFOX_CLIENT_ID $ENV{AVITAB_APIKEY_CHARTFOX})
    message(STATUS "Chartfox API key has been defined.")
endif()

# Host platform definitions applying to everything declared after here
if(WIN32) # targetting Windows systems
    add_compile_definitions(AVITAB_WINDOWS _USE_MATH_DEFINES)
    if(MSVC)
        add_compile_definitions(NOMINMAX)
    endif()
elseif(APPLE) # targetting Apple MacOS systems
    add_compile_definitions(AVITAB_MACOS)
elseif(LINUX OR UNIX) # targetting Linux systems
    add_compile_definitions(AVITAB_LINUX)
endif()

## This custom tool is used to compile SVG icons and is declared at this outer
## level as it is referenced in the resources sub-tree below.
#add_executable(avitab_svgc)

# Avitab common source code is here
add_subdirectory(src)

# Avitab resources (icons, etc?) are here
add_subdirectory(res)

# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
if(BUILD_TESTING AND (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING))
    add_subdirectory(tests)
endif()

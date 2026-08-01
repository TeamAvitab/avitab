# This file is part of the Avitab project. See the README and LICENSE for details.

configure_file(config.h.in "${PROJECT_BINARY_DIR}/AviTabBuildSettings.h")

# The Avitab sources are built into one core library for all product, and
# simulator-specific libraries for X-Plane, MSFS and the mock
# simulator (for development and test), and UI-window libraries.
# Each product will link the core library, and whichever simulation
# and UI-window libraries are appropriate.

include_directories("${CMAKE_CURRENT_LIST_DIR}" "${PROJECT_BINARY_DIR}")

# All of the core Avitab classes are put in here
add_library(avitab_core_lib)
target_include_directories(avitab_core_lib
    PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/api"
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/core"
)
target_link_libraries(avitab_core_lib PUBLIC
    avitab_extern
)

# The XPlane library provides interfacing to XPlane for both simulator and window interfaces.
add_library(avitab_xplane_lib)
target_include_directories(avitab_xplane_lib PUBLIC
    "${XPLANE_SDK}/CHeaders"
)
target_compile_definitions(avitab_xplane_lib PUBLIC
    XPLM400 XPLM303 XPLM301 XPLM300 XPLM210 XPLM200
    APL=$<BOOL:${APPLE}> IBM=$<BOOL:${WIN32}> LIN=$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>
)
if(APPLE)
    target_compile_definitions(avitab_xplane_lib PRIVATE GL_SILENCE_DEPRECATION)
endif()
target_link_libraries(avitab_xplane_lib PRIVATE
    avitab_core_lib
)

# The mock library provides a mock simulation and desktop GLFW window for the Avitab standalone.
add_library(avitab_mock_lib)
if(APPLE)
    target_compile_definitions(avitab_mock_lib PRIVATE
        GL_SILENCE_DEPRECATION
    )
endif()
target_link_libraries(avitab_mock_lib PRIVATE
    avitab_core_lib
    glfw3_imp
)

if(MSFS_SDK)
    # The MSFS library provides interfacing to MSFS for both simulator and window interfaces.
    add_library(avitab_msfs_lib)
    target_include_directories(avitab_msfs_lib PRIVATE
        "$ENV{MSFS_SDK}/SimConnect SDK/include"
    )
    add_library(msfs_simconnect_dll SHARED IMPORTED)
    cmake_path(SET MSFS_SDK $ENV{MSFS_SDK})
    set_property(TARGET msfs_simconnect_dll PROPERTY IMPORTED_LOCATION "${MSFS_SDK}/SimConnect SDK/lib/SimConnect.dll")
    set_property(TARGET msfs_simconnect_dll PROPERTY IMPORTED_IMPLIB "${MSFS_SDK}/SimConnect SDK/lib/SimConnect.lib")
    target_link_libraries(avitab_msfs_lib PRIVATE
        avitab_core_lib
        glfw3_imp
        msfs_simconnect_dll
    )
endif()

add_subdirectory(api)
add_subdirectory(core)
add_subdirectory(product)
add_subdirectory(platform)
add_subdirectory(sim)
add_subdirectory(ui)

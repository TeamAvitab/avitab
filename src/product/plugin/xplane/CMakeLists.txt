# This file is part of the Avitab project. See the README and LICENSE for details.

# The Avitab plugin for the X-Plane simulator

add_library(xplane_plugin SHARED)

target_sources(xplane_plugin PRIVATE
    Plugin.cpp
)

target_link_libraries(xplane_plugin PRIVATE
    avitab_core_lib
    avitab_xplane_lib
)

if(WIN32)
    target_compile_definitions(xplane_plugin PRIVATE IBM=1)
    target_link_options(xplane_plugin PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-static -static-libgcc -static-libstdc++>
    )
    target_link_libraries(xplane_plugin PRIVATE
        "${XPLANE_SDK}/Libraries/Win/XPLM_64.lib"
        opengl32
    )
    install(TARGETS xplane_plugin RUNTIME DESTINATION avitab/xplane/Avitab/win_x64)
elseif(APPLE)
    target_compile_definitions(xplane_plugin PRIVATE APL=1)
    target_link_options(xplane_plugin PRIVATE
        "-F${XPLANE_SDK}/Libraries/Mac"
    )
    target_link_libraries(xplane_plugin PRIVATE
        "-framework XPLM"
        "-framework OpenGL"
    )
    install(TARGETS xplane_plugin DESTINATION avitab/xplane/Avitab/mac_x64)
elseif(UNIX)
    target_compile_definitions(xplane_plugin PRIVATE LIN=1)
    target_link_options(xplane_plugin PRIVATE
        -static-libgcc -static-libstdc++
    )
    target_link_libraries(xplane_plugin PRIVATE
        GL
    )
    install(TARGETS xplane_plugin DESTINATION avitab/xplane/Avitab/lin_x64)
endif()

set_target_properties(xplane_plugin PROPERTIES PREFIX "")
set_target_properties(xplane_plugin PROPERTIES OUTPUT_NAME "Avitab")
set_target_properties(xplane_plugin PROPERTIES SUFFIX ".xpl")

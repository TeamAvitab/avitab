# This file is part of the Avitab project. See the README and LICENSE for details.

if(MSFS_SDK)
    add_subdirectory(msfs)
endif()

# The Avitab standalone desktop GUI

add_executable(avitab_desktop)

target_sources(avitab_desktop PRIVATE
    Desktop.cpp
)

target_link_libraries(avitab_desktop PRIVATE
    avitab_glfw_lib
    avitab_core_lib
    avitab_xplane_lib
    glfw3_imp ${CMAKE_DL_LIBS}
)

if(WIN32)
    target_link_options(avitab_desktop PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/ENTRY:mainCRTStartup>
        $<$<CXX_COMPILER_ID:GNU>:-static -static-libgcc -static-libstdc++>
    )
    target_link_libraries(avitab_desktop PRIVATE
        opengl32
    )
elseif(APPLE)
    target_link_libraries(avitab_desktop PRIVATE
        "-framework OpenGL"
    )
elseif(UNIX)
    target_link_libraries(avitab_desktop PRIVATE
        GL
    )
endif()

install(TARGETS avitab_desktop DESTINATION avitab/tools)

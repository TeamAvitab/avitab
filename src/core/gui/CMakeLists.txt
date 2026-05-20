# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    LVGLToolkit.cpp
    LVGLToolkit.h
    Timer.cpp
    Timer.h
)

add_subdirectory(widgets)

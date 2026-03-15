# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    DDSImage.cpp
    DDSImage.h
    Image.cpp
    Image.h
    Rasterizer.cpp
    Rasterizer.h
    TTFStamper.cpp
    TTFStamper.h
    XTiffImage.cpp
    XTiffImage.h
)

add_subdirectory(stitcher)

# This file is part of the Avitab project. See the README and LICENSE for details.

# Tool for rasterising SVG files to make icons for the UI.

target_sources(avitab_svgc PRIVATE
    main.cpp
)
target_compile_definitions(avitab_svgc PRIVATE
    ${AVITAB_lunasvg_CPPDEFS}
)
target_include_directories(avitab_svgc PRIVATE
    ${AVITAB_lunasvg_INCDIRS}
)
target_link_libraries(avitab_svgc PRIVATE
    lunasvg_imp
)

set_target_properties(avitab_svgc PROPERTIES OUTPUT_NAME "svgc")

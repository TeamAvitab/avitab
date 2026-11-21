# This file is part of the Avitab project. See the README and LICENSE for details.

# Avitab uses some custom icons which are generated from SVG definitions.
# These SVG definitions are compiled to embedded bitmaps which are drawn
# directly onto the image bitmaps for runtime efficiency. There is a custom
# tool to compile the SVG sources which is built as part of the Avitab build
# system.

set(SVGC_SOURCES
    # This style of encoding allows specification of the generated bitmap
    # width and height in the pseudo source file name. The actual SVG source
    # file name is everything before the final 2 extensions!
    avitab.svg.40.40
)

set(SVG_TARGETS)

foreach(work ${SVGC_SOURCES})
    get_filename_component(height "${work}" LAST_EXT)
    string(SUBSTRING ${height} 1 -1 height)
    get_filename_component(work "${work}" NAME_WLE)
    get_filename_component(width "${work}" LAST_EXT)
    string(SUBSTRING ${width} 1 -1 width)
    get_filename_component(svg_source "${work}" NAME_WLE)
    get_filename_component(base "${svg_source}" NAME_WLE)
    set(file_h "${PROJECT_BINARY_DIR}/svg/${base}_${width}x${height}.h")
    list(APPEND SVG_TARGETS "${file_h}")
    add_custom_command(
            OUTPUT "${file_h}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/svg"
            COMMAND $<TARGET_FILE:avitab_svgc> "-o${file_h}" -w${width} -h${height} "${CMAKE_CURRENT_LIST_DIR}/${svg_source}"
            DEPENDS "${svg_source}" avitab_svgc
            VERBATIM
    )
endforeach()

add_custom_target(avitab_svg_headers DEPENDS ${SVG_TARGETS})

# This file is part of the Avitab project. See the README and LICENSE for details.

# GLFW project - provides OpenGL window manager, etc

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)

add_subdirectory("${AVITAB_glfw_SOURCE_DIR}" "${AVITAB_glfw_BINARY_DIR}" EXCLUDE_FROM_ALL)
add_library(glfw3 ALIAS glfw)

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_glfw_PKGLIBS glfw3 CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_glfw3_LOCATION "${AVITAB_LIBS3RD_DIR}/glfw3.lib" CACHE INTERNAL "")
else()
    set(AVITAB_glfw3_LOCATION "${AVITAB_LIBS3RD_DIR}/libglfw3.a" CACHE INTERNAL "")
endif()
set(AVITAB_glfw3_INCDIRS "${AVITAB_glfw_SOURCE_DIR}/include" CACHE INTERNAL "")

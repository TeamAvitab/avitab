# This file is part of the Avitab project. See the README and LICENSE for details.

# Avitab uses Clip's CMake scripts.

set(CLIP_ENABLE_IMAGE OFF)
set(CLIP_ENABLE_LIST_FORMATS OFF)
# Disable clip examples and tests
set(CLIP_EXAMPLES OFF)
set(CLIP_TESTS OFF)
set(CLIP_X11_WITH_PNG OFF)

add_subdirectory("${AVITAB_clip_SOURCE_DIR}" "${AVITAB_clip_BINARY_DIR}" EXCLUDE_FROM_ALL)

if(MSVC)
    set(CLIP_LIBRARY "${AVITAB_LIBS3RD_DIR}/clip.lib" CACHE INTERNAL "")
else()
    set(CLIP_LIBRARY "${AVITAB_LIBS3RD_DIR}/libclip.a" CACHE INTERNAL "")
endif()

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_clip_PKGLIBS clip CACHE INTERNAL "")
set(AVITAB_clip_LOCATION "${CLIP_LIBRARY}" CACHE INTERNAL "")
set(AVITAB_clip_INCDIRS "${AVITAB_clip_SOURCE_DIR}" CACHE INTERNAL "")

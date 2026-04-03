# This file is part of the Avitab project. See the README and LICENSE for details.

# NLohmann's JSON library

add_subdirectory("${AVITAB_json_SOURCE_DIR}" "${AVITAB_json_BINARY_DIR}" EXCLUDE_FROM_ALL)

# Cache settings to be used when AVITAB_FORCE_BUILD_THIRDPARTY is OFF
set(AVITAB_json_PKGLIBS nlohmann_json CACHE INTERNAL "")
set(AVITAB_nlohmann_json_INCDIRS "${AVITAB_json_SOURCE_DIR}/include" CACHE INTERNAL "")

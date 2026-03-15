# This file is part of the Avitab project. See the README and LICENSE for details.

# Overwite the default CMakeLists.txt with one that's tailored for Avitab.
file(COPY_FILE "${CMAKE_CURRENT_LIST_DIR}/patches/CMakeLists.txt" "${AVITAB_harfbuzz_SOURCE_DIR}/CMakeLists.txt")

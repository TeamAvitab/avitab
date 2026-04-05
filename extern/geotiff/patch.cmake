# This file is part of the Avitab project. See the README and LICENSE for details.

# Patch the default CMakeLists.txt with one that's tweaked for Avitab.
file(READ "${AVITAB_geotiff_SOURCE_DIR}/libgeotiff/CMakeLists.txt" FILE_CONTENTS)

string(REPLACE "(VERSION 2.6.0)" "(VERSION 3.23.0)" FILE_CONTENTS "${FILE_CONTENTS}")

file(WRITE "${AVITAB_geotiff_SOURCE_DIR}/libgeotiff/CMakeLists.txt" "${FILE_CONTENTS}")

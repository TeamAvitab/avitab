# This file is part of the Avitab project. See the README and LICENSE for details.

# Overwite the default config.h with one that's tailored for Avitab.
file(COPY_FILE "${CMAKE_CURRENT_LIST_DIR}/patches/config.h" "${AVITAB_mupdf_SOURCE_DIR}/include/mupdf/fitz/config.h")

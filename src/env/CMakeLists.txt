# This file is part of the Avitab project. See the README and LICENSE for details.

add_subdirectory(xplane)
if(MSFS_SDK)
    add_subdirectory(msfs)
endif()
add_subdirectory(standalone)

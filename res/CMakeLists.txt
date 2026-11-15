# This file is part of the Avitab project. See the README and LICENSE for details.

add_subdirectory(svg)

install(DIRECTORY charts icons MapTiles DESTINATION avitab/xplane/Avitab)
install(DIRECTORY charts icons DESTINATION avitab/tools)

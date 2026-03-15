# This file is part of the Avitab project. See the README and LICENSE for details.

macro(install_resources TARGDIR)
    install(FILES config.json DESTINATION ${TARGDIR})
    install(DIRECTORY charts icons online-maps DESTINATION ${TARGDIR})
endmacro()

install_resources(avitab/tools)
install_resources(avitab/xplane/Avitab)

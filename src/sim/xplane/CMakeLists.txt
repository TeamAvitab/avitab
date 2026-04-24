# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_xplane_lib PRIVATE
    DataCache.cpp
    DataCache.h
    DataRefExport.cpp
    DataRefExport.h
    DataRefImport.cpp
    DataRefImport.h
    MonitorBoundsDecider.cpp
    MonitorBoundsDecider.h
    XPlaneEnvironment.cpp
    XPlaneEnvironment.h
    XPlaneGUIDriver.cpp
    XPlaneGUIDriver.h
)

add_subdirectory(libxdata)

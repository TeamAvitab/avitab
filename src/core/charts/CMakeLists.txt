# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    APICall.h
    Chart.h
    ChartService.cpp
    ChartService.h
    Crypto.cpp
    Crypto.h
    RESTClient.cpp
    RESTClient.h
)

add_subdirectory(libchartfox)
add_subdirectory(liblocalfile)
add_subdirectory(libnavigraph)

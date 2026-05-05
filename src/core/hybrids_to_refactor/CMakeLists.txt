# This file is part of the Avitab project. See the README and LICENSE for details.

# REFACTOR - The sources in this directory are a confusing mess of spaghetti.
# They should really be in the simulation or windows drivers, but have deep
# hooks into the core that need to be abstracted. That's too much work to do
# in one step, so this temporary collection has been created.

target_sources(avitab_core_lib PRIVATE
    # somewhat related to the simulation driver
    EnvData.h
    Environment.cpp
    Environment.h
    ToolEnvironment.cpp
    ToolEnvironment.h

    # somewhat related to the window driver
    GUIDriver.cpp
    GUIDriver.h

    # somewhat related to the world nav database
    LoadManager.cpp
    LoadManager.h
    FMSLoader.cpp
    FMSLoader.h
    UserFixLoader.cpp
    UserFixLoader.h
    BaseParser.cpp
    BaseParser.h
    FMSParser.cpp
    FMSParser.h
    UserFixParser.cpp
    UserFixParser.h
    FlightPlanNodeData.h
    UserFixData.h
)

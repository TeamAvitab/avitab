# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    NavDbSchema.h
    SqlDatabase.cpp
    SqlDatabase.h
    SqlLoadManager.cpp
    SqlLoadManager.h
    SqlStatement.cpp
    SqlStatement.h
    SqlWorld.cpp
    SqlWorld.h
)

add_subdirectory(loaders)
add_subdirectory(models)

# This file is part of the Avitab project. See the README and LICENSE for details.

target_sources(avitab_core_lib PRIVATE
    InMemNavDb.cpp
    InMemNavDb.h
    NavDbManager.cpp
    NavDbManager.h
)

add_subdirectory(loaders)
add_subdirectory(models)
add_subdirectory(parsers)
add_subdirectory(routing)
add_subdirectory(other)

# Building Avitab from source

The Avitab build scripts have been changed significantly since the 0.7.1 release (this was the final release from Avitab's original author and maintainer, Folke Will).

This document explains the updated build processes.

## Quick Start

You will first need:
- A clone of the Avitab repository **github.com/TeamAvitab/avitab**.

You can then:
- Configure the build scripts with `cmake --preset release`.
- Build everything with `cmake --build --preset release`.

The directory `install` will be created containing the X-Plane plugin package and a desktop application which can be used for basic GUI testing. The X-Plane plugin package can be copied into your X-Plane installation for full in-simulation testing.

> If you are using an IDE that supports CMake then you can usually open the Avitab root directory in the IDE and it will automatically configure the build scripts.

> Configuration of some of the 3rd-party libraries require additional tools that may not be installed as base packages on all development systems. Missing packages should be identifiable from the error log if configuration of the project fails, and can be installed using your package manager.

> Currently not all compiler toolsets are supported. The traditional Avitab toolchains of gcc (Linux), MinGW (Windows) and clang (MacOS) should be OK. The Avitab team aim to add MSVC support in the near future.

> The first configure and build will take some time. After a successful initial build run `cmake --preset release` again. An optimised build graph will be created and subsequent builds will be much faster.

Read on if you require further detailed explanation.

## Why the build system has been changed

The Avitab team is working towards supporting a wider range of compilers and toolsets and improving the build system. To assist with this goal, the CMake build scripts have been completely updated.

## How the build system has changed

Avitab uses a large number of 3rd-party libraries to provide a significant amount of its functionality. Previously these libraries were incorporated into the Avitab source repository as git submodules. The 3rd-party libraries were built manually (typically just once) by running a bash shell script, which restricted toolchain support to Unix-like environments.

With the recent build script update the download, configuration and build of the 3rd-party libraries is incorporated into the main Avitab CMake build scripts.

The Avitab git repository no longer uses git submodules. Instead a snapshot of each 3rd-party library's source tree is downloaded into the Avitab build tree (if needed) during the CMake build configuration, and each 3rd-party library is configured for building within the top-level Avitab project.

Downloading only the chosen release for each 3rd-party library is quicker and uses less disk space than the full library repository.

Once the 3rd-party libraries have been built there is rarely any need to rebuild them. The CMake build configuration can be re-run after the first successful complete build; it will treat the 3rd-party libraries as installed binaries and only include Avitab's source files in the build. If a full build is ever required, then delete the CMake cache file and then reconfigure the build.

## How the build directory is organised

Two special directories are created at the top-level of the main **build** directory.

**build/download** contains the 3rd-party library packages that were downloaded and their expanded source trees. Signature files are used to avoid fetching packages that are already available. You should rarely want/need to delete this directory.

**build/libs3rd** contains the 3rd-party library build products. It is, in effect, a separate top-level build directory for the 3rd-party libraries, allowing them to be kept separate from the Avitab build in case you're a fan of cleaning a build by deleting the build directory, but don't really want to start completely from scratch!

Regular build trees are created for *debug* and *release* configurations, as well as other presets that have been defined (currently *xcode* since the IDE cannot open a CMake project natively). These folders can be deleted if required to create an almost-clean build of Avitab (the 3rd-party libraries will be included in the regenerated build graph, but should not require rebuilding).

## How the 3rd-party libraries are hooked into the build

Many of the 3rd-party libraries have CMake support that is sufficiently up to date that the project's scripts can be directly incorporated into the Avitab script tree.

Some of the 3rd-party libraries have CMake support which is not designed for inclusion into another project. In this case either patches are applied after the library has been downloaded and unpacked, or the project's CMake scripts are ignored and a basic CMake build wrapper has been written to build the library sources directly.

A small number of 3rd-party libraries do not have any CMake support. A simple CMake build wrapper has been written to build the library sources as required for Avitab.

> The 3rd-party download, unpack, and build hooking scripts are an ad hoc solution for Avitab. However, there may be some useful general-purpose CMake library package that can be derived from this, and further modifications to this part of the build system should be expected. (Version 2 is always an improvement!)

## Contributing bug fixes and enhancements

Contributions to Avitab are welcomed.

Contributors are invited to fork the primary repository at **github.com/TeamAvitab/avitab**. We recommend regular synchronisation of (at least) the **main** branch from the primary repository.

We recommend creating a new branch for each PR submission, and rebasing this on the tip of **main** before PR submission.

The Avitab primary repository may also have a number of feature branches intended for more significant updates. For contributions to unintegrated features, **main** should be replaced by the relevant feature branch.

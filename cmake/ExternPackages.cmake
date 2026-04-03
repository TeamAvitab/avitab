# This file is part of the Avitab project. See the README and LICENSE for details.

# This is Avitab's ad hoc equivalent of FetchContent.
# It supports a local cache of all the downloaded and extracted packages to avoid unnecessary
# and duplicated downloads, particularly during development of Avitab core libraries.
# In some ways it's a shallow, simplified version of git's submodule. (But in other ways, not.)


function(IncludeDownloadedPackage name url out_of_date_var)
    # set up some standard paths for this package
    cmake_path(APPEND cachedir "${AVITAB_DOWNLOAD_DIR}" "_cache")
    cmake_path(APPEND pkgurlsig "${cachedir}" "${name}_url")
    cmake_path(APPEND pkgfilesig "${cachedir}" "${name}_file")
    cmake_path(APPEND pkgdirsig "${cachedir}" "${name}_dir")

    # downloading and unpacking can be skipped if the url is unchanged and the
    # downloaded file is ok and the extracted srcdir contains the avitab signature.
    # do all these checks and any cleaning up of previous downloads if required.
    set(url_ok FALSE)
    set(file_ok FALSE)
    set(dir_ok FALSE)

    execute_process(COMMAND "${CMAKE_COMMAND}" -E cat "${pkgurlsig}" OUTPUT_VARIABLE prevurl RESULT_VARIABLE ures ERROR_QUIET)
    if(${ures} EQUAL 0 AND "${url}" STREQUAL "${prevurl}")
        set(url_ok TRUE)
    endif()

    execute_process(COMMAND "${CMAKE_COMMAND}" -E cat "${pkgfilesig}" OUTPUT_VARIABLE prevfile RESULT_VARIABLE fres ERROR_QUIET)
    if(${fres} EQUAL 0 AND EXISTS "${prevfile}")
        set(file_ok TRUE)
    endif()

    execute_process(COMMAND "${CMAKE_COMMAND}" -E cat "${pkgdirsig}" OUTPUT_VARIABLE prevdir RESULT_VARIABLE dres ERROR_QUIET)
    if(${dres} EQUAL 0 AND IS_DIRECTORY "${prevdir}")
        execute_process(COMMAND "${CMAKE_COMMAND}" -E cat "${prevdir}/_sig_avitab" RESULT_VARIABLE sres ERROR_QUIET)
        if(${sres} EQUAL 0)
            set(dir_ok TRUE)
        endif()
    endif()

    if(NOT url_ok)
        # if the requested url doesn't match the previous one then clean up the old
        # signatures and archive file and extracted directory
        file(REMOVE "${pkgurlsig}" "${pkgfilesig}" "${pkgdirsig}")
        if(file_ok)
            file(REMOVE "${prevfile}")
            set(file_ok FALSE)
        endif()
        if(dir_ok)
            file(REMOVE_RECURSE "${prevdir}")
            set(dir_ok FALSE)
        endif()
    elseif(NOT file_ok)
        # the URL hasn't changed, but if the archive file is missing, clean up the extracted directory
        file(REMOVE "${pkgfilesig}" "${pkgdirsig}")
        if(dir_ok)
            file(REMOVE_RECURSE "${prevdir}")
            set(dir_ok FALSE)
        endif()
    endif()

    # clean up of previous artefacts should now have been done
    if(file_ok)
        set(pkgfile "${prevfile}")
    else()
        # need to download the archive
        cmake_path(GET url FILENAME fn)
        set(fname "${name}-${fn}")
        cmake_path(APPEND pkgfile "${cachedir}" "${fname}")
        file(MAKE_DIRECTORY "${cachedir}")
        message(STATUS "Downloading ${name} : ${fname} <- ${url}")
        file(DOWNLOAD "${url}" "${pkgfile}" STATUS s)
        list(POP_FRONT s dlres dlerrstring)
        if(${dlres} EQUAL 0)
            # download was OK, so we can create the signatures to avoid doing this again
            file(WRITE "${pkgurlsig}" "${url}")
            file(WRITE "${pkgfilesig}" "${pkgfile}")
        else()
            message(FATAL_ERROR "Download of ${name} required package failed (${dlerrstring}).")
            file(REMOVE "${pkgfile}")
        endif()
    endif()

    if(dir_ok)
        set(pkgsrcpath "${prevdir}")
    else()
        # need to unpack the archive
        # this will be the name of the package source root directory
        cmake_path(APPEND pkgsrcpath "${AVITAB_DOWNLOAD_DIR}" "${name}")
        # get a list of the files in the package
        execute_process(COMMAND "${CMAKE_COMMAND}" -E tar tf "${pkgfile}" WORKING_DIRECTORY "${cachedir}" OUTPUT_FILE _tmp.txt)
        file(STRINGS "${cachedir}/_tmp.txt" pkgcontents)
        foreach(f ${pkgcontents})
            # get the first part of the path
            string(REGEX REPLACE "/.*$" "" f "${f}")
            list(APPEND tops "${f}")
        endforeach()
        list(REMOVE_DUPLICATES tops)
        list(LENGTH tops ndirs)
        if("${ndirs}" EQUAL 1)
            # archive has one top-level directory, we can unpack directly
            execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xf "${pkgfile}" WORKING_DIRECTORY "${AVITAB_DOWNLOAD_DIR}")
            list(POP_FRONT tops pkgtop)
            cmake_path(APPEND pkgtop "${AVITAB_DOWNLOAD_DIR}" "${pkgtop}")
            file(REMOVE_RECURSE "${pkgsrcpath}")
            #execute_process(COMMAND "${CMAKE_COMMAND}" -E rename "${pkgtop}" "${pkgsrcpath}")
            file(RENAME "${pkgtop}" "${pkgsrcpath}")
        else()
            # archive has multiple top-level items, unpack into a subdirectory
            execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${pkgsrcpath}")
            execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xf "${pkgfile}" WORKING_DIRECTORY "${pkgsrcpath}")
        endif()

        # run the patch script, if there is one
        set(AVITAB_${name}_SOURCE_DIR "${pkgsrcpath}")
        include("${name}/patch.cmake" OPTIONAL)

        # write the files that will bypass these steps next time.
        file(WRITE "${pkgdirsig}" "${pkgsrcpath}")
        file(WRITE "${pkgsrcpath}/_sig_avitab" "")
    endif()

    # The package has been downloaded and extracted and the source is in ${pkgsrcpath}
    # set some (parent) variables to provide source/build directores for the package
    set(AVITAB_${name}_SOURCE_DIR "${pkgsrcpath}" PARENT_SCOPE)
    set(AVITAB_${name}_BINARY_DIR "${PROJECT_BINARY_DIR}/download/${name}" PARENT_SCOPE)

    # If the package didn't need downloading/unpacking, and its libraries and include paths
    # are already recorded in the CMake cache, and those libraries and include paths look ok,
    # then we'll treat this as a prebuilt imported library that does not need configure and build.
    set(needs_conf_and_build TRUE)
    if(dir_ok AND AVITAB_${name}_PKGLIBS)
        set(needs_conf_and_build FALSE)
        foreach(lib ${AVITAB_${name}_PKGLIBS})
            if(NOT "${AVITAB_${lib}_LOCATION}" STREQUAL "")
                if(NOT EXISTS "${AVITAB_${lib}_LOCATION}")
                    set(needs_conf_and_build TRUE)
                endif()
            endif()
            foreach(idir ${AVITAB_${lib}_INCDIRS})
                if(NOT IS_DIRECTORY "${idir}")
                    set(needs_conf_and_build TRUE)
                endif()
            endforeach()
        endforeach()
    endif()
    set(${out_of_date_var} ${needs_conf_and_build} PARENT_SCOPE)
endfunction()

function(PrepareExternPackages)
    # variable number of arguments, but should alway be in pairs, (name, url)
    set(allpkgs)
    set(oodpkgs)
    foreach(x ${ARGV})
        if("${pkg}" STREQUAL "")
            set(pkg ${x})
        else()
            IncludeDownloadedPackage(${pkg} "${x}" ood)
            set(allpkgs ${allpkgs} ${pkg})
            if(ood)
                set(oodpkgs ${oodpkgs} ${pkg})
            endif()
            set(AVITAB_${pkg}_SOURCE_DIR "${AVITAB_${pkg}_SOURCE_DIR}" PARENT_SCOPE)
            set(AVITAB_${pkg}_BINARY_DIR "${AVITAB_${pkg}_BINARY_DIR}" PARENT_SCOPE)
            unset(pkg)
        endif()
    endforeach()
    set(AVITAB_ALL_PACKAGES ${allpkgs} PARENT_SCOPE)
    set(AVITAB_OUT_OF_DATE_PACKAGES ${oodpkgs} PARENT_SCOPE)
endfunction()

# This function includes the CMake scripts for each of the external libraries.
# A wrapper script is required for each library, used to set up the library build
# options and set some cache variables that will be required later.
function(ConfigureExternPackages)
    foreach(pkg ${ARGV})
        message(STATUS "Configuring build for Avitab third-party package ${pkg}")
        add_subdirectory("${pkg}")
    endforeach()
endfunction()

# This function 'installs' (copies) the third-party packages' libraries
# to a common build directory, and gives each library a well-known name.
function(InstallExternLibraries)
    # Generate rules to copy each of the third-party libraries into the build directory.
    set(installed_libs)
    foreach(lib ${ARGV})
        if(NOT "${AVITAB_${lib}_LOCATION}" STREQUAL "")
            if(MSVC)
                set(outlib "${AVITAB_LIBS3RD_DIR}/${lib}.lib")
            else()
                set(outlib "${AVITAB_LIBS3RD_DIR}/lib${lib}.a")
            endif()
            add_custom_command(
                OUTPUT "${outlib}"
                COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${lib}>" "${outlib}"
                DEPENDS ${lib}
                VERBATIM
            )
            add_library(${lib}_install INTERFACE "${outlib}")
            set(installed_libs ${installed_libs} ${outlib})
        endif()
    endforeach()
    set(AVITAB_ALL_INSTALLED_LIBS ${installed_libs} PARENT_SCOPE)
endfunction()

# This function 'imports' the third-party packages' libraries, and handles the
# situation where the library is being built from source as part of the build
# and the situation where the library was built previously and has not been
# included in the cmake build tree.
function(ImportThirdPartyLibrary name)
    set(implib ${name}_imp)
    if(TARGET ${name}_install)
        # The third-party installed library is a build target.
        # Declare the 'import' library as an interface target and hook in the installed library.
        add_library(${implib} INTERFACE)
        target_link_libraries(${implib} INTERFACE "${AVITAB_${name}_LOCATION}")
    else()
        # The third-party library is not part of this build tree. So we treat it as a pre-built import.
        if("${AVITAB_${name}_LOCATION}" STREQUAL "")
            # library isn't actually built - just an interface target
            add_library(${implib} INTERFACE IMPORTED GLOBAL)
        else()
            # library has an installed location, it should be on the disk
            if(NOT ${AVITAB_FORCE_BUILD_THIRDPARTY} AND NOT EXISTS "${AVITAB_${name}_LOCATION}")
                message(WARNING "AVITAB_FORCE_BUILD_THIRDPARTY has been disabled, but library ${AVITAB_${name}_LOCATION} does not appear to exist. The build may fail.")
            endif()
            add_library(${implib} STATIC IMPORTED GLOBAL)
            set_target_properties(${implib} PROPERTIES IMPORTED_LOCATION "${AVITAB_${name}_LOCATION}")
        endif()
    endif()
    # include dirs
    if(NOT "${AVITAB_${name}_INCDIRS}" STREQUAL "")
        target_include_directories(${implib} INTERFACE ${AVITAB_${name}_INCDIRS})
    endif()
    # preproc definitions
    if(NOT "${AVITAB_${name}_CPPDEFS}" STREQUAL "")
        target_compile_definitions(${implib} INTERFACE ${AVITAB_${name}_CPPDEFS})
    endif()
endfunction()

# This function creates an imported library target

function(ImportExternLibraries)
    foreach(lib ${ARGV})
        ImportThirdPartyLibrary("${lib}")
    endforeach()
    # declare the third party library interdependencies only after they have all been imported
    foreach(lib ${ARGV})
        foreach(tll ${AVITAB_${lib}_LINKLIBS})
            target_link_libraries(${lib}_imp INTERFACE ${tll}_imp)
        endforeach()
    endforeach()
endfunction()

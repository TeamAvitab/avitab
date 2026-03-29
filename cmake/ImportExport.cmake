# This file is part of the Avitab project. See the README and LICENSE for details.

# This function 'exports' (copies) the third-party packages' libraries
# to a common location, and gives each library a well-known name.
function(ExportThirdPartyLibrary name pathvar)
    if(NOT "${AVITAB_${name}_LOCATION}" STREQUAL "")
        if(MSVC)
            set(outlib "${AVITAB_LIBS3RD_DIR}/${name}.lib")
        else()
            set(outlib "${AVITAB_LIBS3RD_DIR}/lib${name}.a")
        endif()
        add_custom_command(
            OUTPUT "${outlib}"
            COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${name}>" "${outlib}"
            DEPENDS ${name}
            VERBATIM
        )
        add_library(${name}_imp INTERFACE "${outlib}")
        set(${pathvar} ${outlib} PARENT_SCOPE)
    endif()
endfunction()

# This function 'imports' the third-party packages' libraries, and handles the
# situation where the library is being built from source as part of the build
# and the situation where the library was built previously and has not been
# included in the cmake build tree.

function(ImportThirdPartyLibrary name)
    set(impname ${name}_imp)
    if(NOT TARGET ${impname})
        # The third-party library is not part of this build tree. So we treat it as a pre-built import.
        if("${AVITAB_${name}_LOCATION}" STREQUAL "")
            add_library(${impname} INTERFACE IMPORTED GLOBAL)
        else()
            if(NOT ${BUILD_AVITAB_THIRDPARTY} AND NOT EXISTS "${AVITAB_${name}_LOCATION}")
                message(WARNING "BUILD_AVITAB_THIRDPARTY has been disabled, but library ${AVITAB_${name}_LOCATION} does not appear to exist. The build may fail.")
            endif()
            add_library(${impname} STATIC IMPORTED GLOBAL)
            set_target_properties(${impname} PROPERTIES IMPORTED_LOCATION "${AVITAB_${name}_LOCATION}")
        endif()
    else()
        # The third-party library is being built, so just declare a link dependency on that.
        target_link_libraries(${impname} INTERFACE ${AVITAB_${name}_LOCATION})
    endif()
    # include dirs
    if(NOT "${AVITAB_${name}_INCDIRS}" STREQUAL "")
        target_include_directories(${impname} INTERFACE ${AVITAB_${name}_INCDIRS})
    endif()
    # preproc definitions
    if(NOT "${AVITAB_${name}_CPPDEFS}" STREQUAL "")
        target_compile_definitions(${impname} INTERFACE ${AVITAB_${name}_CPPDEFS})
    endif()
endfunction()

#######################################################
## Tools for re-packaging existing third party code and 
## re-using existing binaries in the build process
#######################################################
######
# Here we reset the CPACK Configuration and re-initalize it to our needs
######
include(get_compiler_arch)

function(package_project pro)
    # find current build configuration, required because install() on msbuild
    # does not specify the proper config, we enforce setting CMAKE_BUILD_TYPE
    set(_pro_cfg ${CMAKE_BUILD_TYPE})
    if (NOT "${CMAKE_BUILD_TYPE}" MATCHES "Debug" 
        AND NOT "${CMAKE_BUILD_TYPE}" MATCHES "Release")
        message(FATAL_ERROR "please set -DCMAKE_BUILD_TYPE to Release or  Debug")
    endif()
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}" CACHE STRING "Fixated build type for installation" FORCE)
    #find cpack executable
    get_filename_component(_cp_dir ${CMAKE_COMMAND} DIRECTORY)
    find_program(_cp_command cpack
        PATHS ${_cp_dir}
        NO_DEFAULT_PATH
        DOC "searching CPACK executable"
    )
    if(NOT _cp_command)
        message(FATAL_ERROR "cannot find cpack executable! (searched in ${_cp_dir}")
    endif()
    set(_pro_workdir ${CMAKE_BINARY_DIR}/package/${pro})
    #TODO set/get  version from target properties
    if(NOT TARGET ${pro})
        message(FATAL_ERROR "package_project: target does not exist: ${pro}")
    endif()
    get_compiler_arch(_comp _arch _plat)
    get_target_property(PRO_BUILD ${pro} BINARY_DIR)
    if(NOT PRO_BUILD)
        set(PRO_BUILD ${${pro}_BINARY_DIR})
    endif()
    set(PRO_NAME ${pro})
    set(PRO_COMPONENT ALL)
    set(PRO_DIR "/")
    set(PRO_ARCH "${_plat}")
    set(PRO_TOOL "${_comp}")
    set(PRO_PKG_FMT "ZIP")
    configure_file(${IntegrationBus_SOURCE_DIR}/cmake/CPackConfig.cmake.in ${_pro_workdir}/CPackConfig.cmake @ONLY)
    set(PRO_FILE_NAME "${PRO_NAME}-${PROJECT_VERSION}-${PRO_TOOL}-${_pro_cfg}-${PRO_ARCH}")
    add_custom_target(package-${pro}
        COMMAND ${_cp_command}  -C "${_pro_cfg}"
        ALL
        WORKING_DIRECTORY ${_pro_workdir}
        COMMENT "Running cpack for ${pro} for build config ${_pro_cfg}"
        BYPRODUCTS "${_pro_workdir}/${PRO_FILE_NAME}"
    )
    add_dependencies(package-${pro} ${pro})
endfunction()

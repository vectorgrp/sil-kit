#######################################################################################################################
# Configure the IntegrationBus packaging: Group multiple configurations into a single package
#######################################################################################################################

# CPack does not inherently add multiple build targets 'Debug' and 'Release' into a single package
# For VisualStudio as a multi-config generator, we would like to support this case.
#   $ mkdir build_x64_Debug   && cd build_x64_Debug   && cmake .. -A x64 && cmake --build . --config Debug
#   $ mkdir build_x64_Release && cd build_x64_Release && cmake .. -A x64 && cmake --build . --config Release
# Note that, building both configs into the same folder is not supported by CPack, and you must not use the -C argument on the call below:
#   $ mkdir packages && cd packages && cpack --config ../build_x64_Debug/CPackConfig.cmake --verbose
# Sources of inspiration:
# * https://cmake.org/pipermail/cmake/2013-February/053508.html
# * https://www.reddit.com/r/cpp/comments/8sie4b/i_manage_the_release_cycle_for_cmake_the_build/e0zmr7d/

option(CPACK_MULTICONFIG_PACKAGE "Enable creating multi-config packages from multiple build folders" OFF)

macro(configure_ib_multiconfig_package)
    if(CPACK_MULTICONFIG_PACKAGE)
        message(STATUS "Configuring multi-config packages from multiple build folders")

        if(CMAKE_SYSTEM_NAME MATCHES "Linux")
            # Linux case: Package only x64 architecture and both configurations (Debug and Release) into one archive, with one top-level folder per configuration
            if(NOT DEFINED CPACK_MULTICONFIG_BUILDFOLDERS)
                set(CPACK_MULTICONFIG_BUILDFOLDERS
                    "../build_x64_Release"
                    "../build_x64_Debug"
                )
                set(CPACK_MULTICONFIG_PACKAGEFOLDERS
                    "/Release"
                    "/Debug"
                )
                message(STATUS "Using a default multi-config set: cmake -DCPACK_MULTICONFIG_BUILDFOLDERS=\"${CPACK_MULTICONFIG_BUILDFOLDERS}\" -DCPACK_MULTICONFIG_PACKAGEFOLDERS=\"${CPACK_MULTICONFIG_PACKAGEFOLDERS}\"")
            endif()
        elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
            # Windows case: Package both architectures (Win32 and x64) and both configurations (Debug and Release) into one archive
            if(NOT DEFINED CPACK_MULTICONFIG_BUILDFOLDERS)
                set(CPACK_MULTICONFIG_BUILDFOLDERS
                    "../build_Win32_Release"
                    "../build_Win32_Debug"
                    "../build_x64_Release"
                    "../build_x64_Debug"
                    )
                set(CPACK_MULTICONFIG_PACKAGEFOLDERS
                    "/"
                    "/"
                    "/"
                    "/"
                )
                message(STATUS "Using a default multi-config set: cmake -DCPACK_MULTICONFIG_BUILDFOLDERS=\"${CPACK_MULTICONFIG_BUILDFOLDERS}\" -DCPACK_MULTICONFIG_PACKAGEFOLDERS=\"${CPACK_MULTICONFIG_PACKAGEFOLDERS}\"")
            endif()
        else()
            message(FATAL_ERROR "System ${CMAKE_SYSTEM_NAME} is not supported as a packaging target")
        endif()

        list(LENGTH CPACK_MULTICONFIG_BUILDFOLDERS count)
        math(EXPR count "${count}-1")
        unset(CPACK_INSTALL_CMAKE_PROJECTS)
        foreach(i RANGE ${count})
            list(GET CPACK_MULTICONFIG_BUILDFOLDERS ${i} buildFolder)
            list(GET CPACK_MULTICONFIG_PACKAGEFOLDERS ${i} packageFolder)

            # Ask CPack to package all build folders defined
            # Quadruplets of "<Build directory>; <CPack project name>; <CPack project component/group>; <Directory in the package>"
            get_filename_component(buildFolderAbsolute "${buildFolder}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
            list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${buildFolderAbsolute};${CMAKE_PROJECT_NAME};ALL;${packageFolder}")
            message(STATUS "Packaging build folder '${buildFolderAbsolute}' into package folder '${packageFolder}'")
        endforeach()

        # Set the configuration for every build, so that the packager (that invokes the generated cmake_install.cmake scripts) knows what to install
        if(NOT CMAKE_BUILD_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$" AND NOT CMAKE_BUILD_TYPE MATCHES "^([De][Ee][Bb][Uu][Gg])$")
            message(FATAL_ERROR "For packaging, you must configure 'cmake -DCMAKE_BUILD_TYPE=Debug' or '-DCMAKE_BUILD_TYPE=Release'")
        endif()
        get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
        if(is_multi_config)
            message(STATUS "Multi-config generator of '${CMAKE_BINARY_DIR}' is restricted to configuration '${CMAKE_BUILD_TYPE}' for packaging")
            set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}" CACHE STRING "Fixated build type for installation" FORCE)
            message(STATUS "Run 'cmake --build . --config ${CMAKE_BUILD_TYPE}'")
        else()
            # If using makefiles, there is nothing else to do
            message(STATUS "Single-config generator of '${CMAKE_BINARY_DIR}' is used for packaging: This configuration is set to '${CMAKE_BUILD_TYPE}'")
        endif()
        # Setting CPACK_BUILD_CONFIG should set the default case of the generated make_install.cmake, but does not seem to work yet.
        # Cf. https://github.com/Kitware/CMake/blob/v3.13.0-rc3/Source/CPack/bills-comments.txt 'PACK_BUILD_CONFIG check this and set the BUILD_TYPE to it'
        #set(CPACK_BUILD_CONFIG "${CMAKE_BUILD_TYPE}" CACHE STRING "Config type to be used for installation of this build" FORCE)
    endif()
endmacro()

# The last value of the quadruplet remains unused in 3.5.1 due to a bug, but was fixed later (fixed with 3.6.3).
# See https://gitlab.kitware.com/cmake/cmake/issues/18320 - CPACK_INSTALL_CMAKE_PROJECTS ignores directory location
# Bug is located in cmake-3.5.1\Source\CPack\cmCPackGenerator.cxx:726-730, realInstallDirectory is never used (intended to be by-ref?)
#   std::string realInstallDirectory = tempInstallDirectory;
#   if ( !installSubDirectory.empty() && installSubDirectory != "/" )
#   {
#       realInstallDirectory += installSubDirectory;
#   }
# HACK: As a workaround, we change the install folders per build iff CPACK_MULTICONFIG_PACKAGE is set
macro(configure_ib_workaround_for_cpack_3_5_multiconfig_bug)
    if(CPACK_MULTICONFIG_PACKAGE)
        if((${CMAKE_VERSION} VERSION_LESS "3.6.0") AND (NOT "${packageFolder}" EQUAL "/") AND (NOT "${packageFolder}" EQUAL ".")) 
            message(WARNING "Workaround to emulate archive subfolders for this CMake prior 3.6.0: Installation is placed in a subfolder")
            # Not working: Value from the single build folder where CPack is called, would be used for all projects
            #set(CPACK_PACKAGING_INSTALL_PREFIX "/${CMAKE_BUILD_TYPE}")
            # The hack: Prepend folder with build configuration ('Debug', 'Release')
            set(INSTALL_LIBDIR ${CMAKE_BUILD_TYPE}/${INSTALL_LIBDIR})
            set(INSTALL_BINDIR ${CMAKE_BUILD_TYPE}/${INSTALL_BINDIR})
            set(INSTALL_INCLUDE_DIR ${CMAKE_BUILD_TYPE}/${INSTALL_INCLUDE_DIR})
            set(INSTALL_SOURCE_DIR ${CMAKE_BUILD_TYPE}/${INSTALL_SOURCE_DIR})
            set(INSTALL_DOCDIR ${CMAKE_BUILD_TYPE}/${INSTALL_DOCDIR})
            set(INSTALL_CODE_DIR ${CMAKE_BUILD_TYPE}/${INSTALL_CODE_DIR})
            set(INSTALL_CONFIG_DIR ${CMAKE_BUILD_TYPE}/${INSTALL_CONFIG_DIR})
        endif()
    endif()
endmacro()

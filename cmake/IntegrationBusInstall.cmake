#######################################################################################################################
# Install definitions
#######################################################################################################################

include(${CMAKE_CURRENT_LIST_DIR}/IntegrationBusPackageMultiConfig.cmake)

macro(configure_ib_install project_name)
    # Install locations
    include(GNUInstallDirs) # GNU-compliant folder names, e.g. ${CMAKE_INSTALL_DATAROOTDIR} = "share"
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
        set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
        set(INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR})
        set(INSTALL_SOURCE_DIR source/${project_name})
        set(INSTALL_DOCDIR ${CMAKE_INSTALL_DOCDIR}) # "share/doc/<project name>"
        set(INSTALL_CODE_DIR ${INSTALL_DOCDIR})

        # This path must be compliant to find_package, which already encodes the configuration into the targets file ('IntegrationBusTargets-${CONFIG_TAG}.cmake')
        set(INSTALL_CONFIG_DIR ${CMAKE_INSTALL_DATAROOTDIR}/cmake/${project_name})
    elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        # Determine environment triplet: target system, bitness, compiler, compiler version, configuration
        if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
            set(SYSTEM_BITNESS "64")
        elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
            set(SYSTEM_BITNESS "32")
        else()
            message(FATAL_ERROR "Bitness is not supported")
        endif()
        set(SYSTEM_TAG "Win${SYSTEM_BITNESS}")

        # On a single-configuration generator like Make: At 'cmake ..', use argument '-DCMAKE_BUILD_TYPE=DEBUG|RELEASE'
        # On a multi-configuration generator like MSBuild: At 'cmake . --build', use argument '--config Debug|Release'
        # https://stackoverflow.com/questions/24460486/cmake-build-type-not-being-used-in-cmakelists-txt
        # https://stackoverflow.com/questions/39135272/how-to-determine-current-build-type-of-visual-studio-with-cmake
        if(CMAKE_BUILD_TYPE MATCHES DEBUG)
            set(CONFIG_TAG "Debug")
        elseif(CMAKE_BUILD_TYPE MATCHES RELEASE)
            set(CONFIG_TAG "Release")
        else()
            # Multi-configuration generator: Evaluation deferred to build-time
            set(CONFIG_TAG "$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>")
        endif()

        set(INSTALL_LIBDIR ${SYSTEM_TAG}/${CONFIG_TAG}/${CMAKE_INSTALL_LIBDIR})
        set(INSTALL_BINDIR ${SYSTEM_TAG}/${CONFIG_TAG}/${CMAKE_INSTALL_BINDIR})
        set(INSTALL_INCLUDE_DIR ${SYSTEM_TAG}/${CMAKE_INSTALL_INCLUDEDIR})
        set(INSTALL_SOURCE_DIR Source/${project_name})
        set(INSTALL_DOCDIR .)
        set(INSTALL_CODE_DIR ${SYSTEM_TAG}/${project_name})

        # This path must be compliant to find_package, which already encodes the configuration into the targets file ('IntegrationBusTargets-${CONFIG_TAG}.cmake')
        set(INSTALL_CONFIG_DIR ${SYSTEM_TAG}/cmake)
    else()
        message(FATAL_ERROR "Platform '${CMAKE_SYSTEM_NAME}' is not supported")
    endif()

    # Apply workaround for creating packages with one top-level folder per build target
    configure_ib_workaround_for_cpack_3_5_multiconfig_bug()
endmacro()

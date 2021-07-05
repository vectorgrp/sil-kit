# ================================================================================
#  WARNING: google test must be added before Fast-CDR / Fast-RTPS
#
#  Otherwise, Fast-CDR or Fast-RTPS will set some CMake Variables that make
#  linking on the google test cases fail with Visual Studio.
# ================================================================================
function(addTestProject)
    # Prevent overriding the parent project's compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # HACK: Temporarily remove the project version to avoid a warning related to unversioned project definitions
    set(PROJECT_VERSION)
    set(PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MINOR)
    set(PROJECT_VERSION_PATCH)

    # Google mock
    add_compile_definitions(UNIT_TEST)
    if(MSVC)
        # Ensure that std::tuple is used instead of std::tr1::tuple
        add_compile_definitions(
            GTEST_LANG_CXX11=1
            _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING=1
        )
    endif()

    message(STATUS "Add subdirectory ThirdParty/googletest-1.8.0")
    add_subdirectory(googletest-1.8.0
        googletest-1.8.0
        EXCLUDE_FROM_ALL
    )

    if(MSVC)
        # Ensure that compile definitions are propagated to gmock users
        target_compile_definitions(gmock
            PUBLIC
            GTEST_LANG_CXX11=1
            _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING=1
        )
        target_compile_definitions(gtest
            PUBLIC
            GTEST_LANG_CXX11=1
            _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING=1
        )
    endif()
endfunction()
addTestProject()

######################################################################
#  FastRTPS  (source or binaries)
######################################################################
set(IB_BIN_FASTRTPS_VERSION 1.8.1)
set(IB_BIN_FASTRTPS_REPOSITORY ) #default: must be user set!

if(IB_BIN_FASTRTPS_ENABLE)
    include(FetchContent)
    include(get_compiler_arch)
    include(GetExternalProject)

    message(STATUS "VIB: fetching binary FastRTPS packages")

    get_compiler_arch(_comp _arch _plat)
    if(${_plat} MATCHES "Linux" )
        get_ubuntu_version(ubuntu)
        set(_plat "ubuntu${ubuntu}")
        if(NOT ${ubuntu} MATCHES "18.04")
            # Use 18.04 stable builds on 20.04
            message(STATUS "NOTE Using binary packages built against Ubuntu 18.04")
            set(_plat "ubuntu18.04")
        endif()
    endif()

    set(binname "fastrtps-${IB_BIN_FASTRTPS_VERSION}-${_plat}-${_comp}")
    if(NOT IB_BIN_FASTRTPS_REPOSITORY)
        message(FATAL_ERROR 
            "You have to set IB_BIN_FASTRTPS_REPOSITORY to an URL serving"
            " the fastrtps zip file named \"${binname}-$<CONFIG>.zip\"."
            " To override the version set IB_BIN_FASTRTPS_VERSION."
            )
    endif()


    FetchContent_Declare(fastrtps-release
        URL ${IB_BIN_FASTRTPS_REPOSITORY}/${binname}-Release.zip
        DOWNLOAD_NO_PROGRESS FALSE
    )
    FetchContent_GetProperties(fastrtps-release)
    if(NOT fastrtps-release_POPULATED)
        message(STATUS "NOTE: fetching pre-built fastrtps Release binaries")
        messagE(STATUS "      from: ${IB_BIN_FASTRTPS_REPOSITORY}/${binname}-Release.zip")
        FetchContent_Populate(fastrtps-release)
    endif()

    FetchContent_Declare(fastrtps-debug
        URL ${IB_BIN_FASTRTPS_REPOSITORY}/${binname}-Debug.zip
        DOWNLOAD_NO_PROGRESS FALSE
    )
    FetchContent_GetProperties(fastrtps-debug)
    if(NOT fastrtps-debug_POPULATED)
        message(STATUS "NOTE: fetching pre-built fastrtps Debug binaries")
        message(STATUS "      from: ${IB_BIN_FASTRTPS_REPOSITORY}/${binname}-Debug.zip")
        FetchContent_Populate(fastrtps-debug)
    endif()

    message(STATUS "VIB: Fast-RTPS downloaded, importing targets")
    #XXX the fastcdr-config.cmake explicitly checks for fastcdr_BIN_DIR
    #    but we have static builds pre-compiled with no */bin folder.
    #    this is a bug in upstream fastrtps cmake.

    set(fastcdr_BIN_DIR "${fastrtps-release_SOURCE_DIR}/bin"
            CACHE STRING "override vendor package config logic" FORCE)
    set(fastrtps_BIN_DIR ${fastcdr_BIN_DIR}
            CACHE STRING "override vendor package config logic" FORCE)
    file(MAKE_DIRECTORY ${fastrtps_BIN_DIR})

    #########

    find_package(fastcdr CONFIG REQUIRED PATHS 
            ${fastrtps-release_SOURCE_DIR}
    )
    find_package(fastrtps CONFIG REQUIRED PATHS 
            ${fastrtps-release_SOURCE_DIR}
    )
    set(fc_deb ${fastrtps-debug_SOURCE_DIR}/lib/libfastcdr.a)
    set(fr_deb ${fastrtps-debug_SOURCE_DIR}/lib/libfastrtps.a)
    if(MSVC)
        set(fc_deb ${fastrtps-debug_SOURCE_DIR}/lib/libfastcdrd-1.0.lib)
        set(fr_deb ${fastrtps-debug_SOURCE_DIR}/lib/libfastrtpsd-1.8.lib)
    endif()
    set_target_properties(fastcdr
        PROPERTIES
        IMPORTED_LOCATION_DEBUG ${fc_deb}
        IMPORTED_GLOBAL true
    )
    set_target_properties(fastrtps
        PROPERTIES
        IMPORTED_LOCATION_DEBUG ${fr_deb}
        IMPORTED_GLOBAL true
    )
    message(STATUS "VIB: Fast-RTPS targets imported")
    #Note: we no longer bundle the pre-built FastRTPS binaries. Users will have
    #to use GIT to fetch the sources, see below.
else()
    ### build from submodule or fetch sources from external mirror or use pre-built binaries (deployed mode)
    option(THIRDPARTY_fastcdr "Activate the use of Fast-RTPS internal thirdparty fastcdr" ON)
    option(THIRDPARTY_Asio "Activate the use of Fast-RTPS internal thirdparty Asio" ON)
    option(TINYXML2_FROM_SOURCE "Integrate TinyXML2 source code inside Fast RTPS" ON)
    option(THIRDPARTY_TinyXML2 "Activate the use of Fast-RTPS internal thirdparty TinyXML2" ON)
    ##################
    #local source tree 
    ##################
    message(STATUS "VIB: Using in-tree Fast-RTPS sources: ${CMAKE_CURRENT_LIST_DIR}")
    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/Fast-RTPS/CMakeLists.txt)
        function(include_fastrtps_with_install_prefix)
            message(STATUS "Building Fast-RTPS from local submodule")
            message(STATUS "Add subdirectory ThirdParty/Fast-RTPS")
            set(BIN_INSTALL_DIR "${INSTALL_BIN_DIR}")
            set(INCLUDE_INSTALL_DIR "${INSTALL_INCLUDE_DIR}")
            set(LIB_INSTALL_DIR "${INSTALL_LIB_DIR}")
            set(DATA_INSTALL_DIR "${INSTALL_DATA_DIR}")
            set(LICENSE_INSTALL_DIR "${INSTALL_DATA_DIR}/Fast-RTPS")
            add_subdirectory(
                Fast-RTPS
                Fast-RTPS
                EXCLUDE_FROM_ALL #make sure this does not install anything during packaging
            )
        endfunction()
        include_fastrtps_with_install_prefix()
    ##################
    # Use git to fetch the Fast-RTPS source tree
    ##################
    else()
        set(FASTRTPS_URL "https://github.com/eProsima/Fast-RTPS")
        set(FASTRTPS_TAG "v${IB_BIN_FASTRTPS_VERSION}")

        message(STATUS "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=")
        message(STATUS "          Fetching missing Fast-RTPS sources via git")
        message(STATUS "          This might take several minutes...")
        message(STATUS "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=")
        message(STATUS " Note: in case of error execute the following steps manually:")
        message(STATUS " Git clone version ${FASTRTPS_TAG} from ${FASTRTPS_URL} into the ThirdParty/Fast-RTPS folder.")
        message(STATUS " For details see the source of ${CMAKE_CURRENT_LIST_FILE}")

        if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_LESS "3.11")
            message("-- Using fallback for old cmake < 3.11")
            if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/external_fastrtps/CMakeLists.txt)
                find_package(Git REQUIRED)
                execute_process(
                    COMMAND "${GIT_EXECUTABLE}" clone -b "${FASTRTPS_TAG}" 
                            --recurse-submodules "${FASTRTPS_URL}" ${CMAKE_CURRENT_BINARY_DIR}/external_fastrtps
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    RESULT_VARIABLE gitErr
                )
                if( NOT "${gitErr}" EQUAL "0")
                    message(FATAL_ERROR "ERROR: git subprocess failed with status: ${gitErr}")
                endif()
            endif()
            add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/external_fastrtps
                Fast-RTPS
                EXCLUDE_FROM_ALL
            )

        else()
            include(FetchContent)
            FetchContent_Declare(extern-fastrtps
                GIT_REPOSITORY ${FASTRTPS_URL}
                GIT_TAG ${FASTRTPS_TAG}
            )
            FetchContent_GetProperties(extern-fastrtps)
            if(NOT extern-fastrtps_POPULATED)
                FetchContent_Populate(extern-fastrtps)
                add_subdirectory(${extern-fastrtps_SOURCE_DIR}
                    Fast-RTPS
                    EXCLUDE_FROM_ALL
                )
            endif()
        endif()
    endif()
endif()


# NB: we use the ASIO shipped with FastRTPS right now
# the binary distribution lacks the Asio sources
# but it should be included/deployed by our packaging tools
set(ASIO_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/Fast-RTPS/thirdparty/asio/asio/include)
set(ASIO_INCLUDE_DIR ${ASIO_INCLUDE_DIR} PARENT_SCOPE)

if(NOT EXISTS ${ASIO_INCLUDE_DIR})
    message(FATAL_ERROR "Asio include directory not found: ${ASIO_INCLUDE_DIR}")
else()
    message(STATUS "Asio include directory found:${ASIO_INCLUDE_DIR}")
endif()


# Third party YAML parsing 
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Disable because of gmock import conflict" FORCE)
add_subdirectory(yaml-cpp
	yaml-cpp
	EXCLUDE_FROM_ALL
	)


# Third Party JSON parsing
add_library(json11
    json11/json11.hpp
    json11/json11.cpp
    )

target_include_directories(json11
    PUBLIC json11
)

# libfmt
function(include_fmt)
    # function() adds an additional scope
    # XXX fmt is in our public ILogger.hpp, and also in the interface link list of IbInterface
    # therefore it *must* be install-exported, otherwise the install-export of Integrationbus fails
    set(FMT_VERSION "6.1.0")
    set(FMT_INSTALL ON CACHE BOOL "Make sure the fmt-header-only interface target is exported" FORCE)
    message(STATUS "Add subdirectory ${CMAKE_CURRENT_LIST_DIR}/fmt-${FMT_VERSION}")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/fmt-${FMT_VERSION}
        ${CMAKE_BINARY_DIR}/ThirdParty/fmt-${FMT_VERSION}
        EXCLUDE_FROM_ALL
    )
endfunction()
include_fmt()

# spdlog
function(include_spdlog)
    # function() adds an additional scope
    # Temporarily patch some CMake variables to trick spdlog's install commands into using our install-folder layout
    set(CMAKE_INSTALL_LIBDIR "${INSTALL_LIB_DIR}")
    set(CMAKE_INSTALL_INCLUDEDIR  "${INSTALL_INCLUDE_DIR}")
    set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    message(STATUS "Add subdirectory ThirdParty/spdlog")
    add_subdirectory(
        spdlog
        ${CMAKE_BINARY_DIR}/ThirdParty/spdlog
        EXCLUDE_FROM_ALL
    )
endfunction()

include_spdlog()


# IDE folders:
# ensure all external projects are grouped in IDE mode
set(yaml_targets yaml-cpp parse read sandbox format)
foreach(_tgt IN LISTS yaml_targets)
    set_property(TARGET ${_tgt} PROPERTY FOLDER "ThirdParty/yaml-cpp")
endforeach()
set_property(TARGET json11 PROPERTY FOLDER "ThirdParty")
set_property(TARGET fastcdr PROPERTY FOLDER "ThirdParty")
set_property(TARGET fastrtps PROPERTY FOLDER "ThirdParty")
set_property(TARGET gmock_main PROPERTY FOLDER "ThirdParty")
set_property(TARGET gtest PROPERTY FOLDER "ThirdParty")

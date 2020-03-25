cmake_minimum_required(VERSION 3.5)

################################################################################
# Project definition
################################################################################

project("IntegrationBus-Library" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)

include(IntegrationBusVersion)
configure_ib_version(${PROJECT_NAME})

set(IB_PROJECT_DIR ${PROJECT_SOURCE_DIR})
set(THIRDPARTY_DIR ${PROJECT_SOURCE_DIR}/../ThirdParty)

option(IB_INSTALL_PDB_FILES "Install Visual Studio debug symbol files along with the IB dll" OFF)

if(MSVC)
    add_compile_options("/MP;/W4;/w44062")
    #disable warnings on visual studio 2015 v14
    if(MSVC_VERSION EQUAL 1900)
        add_compile_options("/wd4503")
    endif()

    #make sure Release is built with debug PDBs
    add_compile_options(/Zi )
    #does not build on CI, seems to be cmake 3.13: add_link_options(/DEBUG)
    #add_link_options(/DEBUG)
    #we add LINK_FLAGS property to IntegrationBus as workaround

endif (MSVC)

add_definitions(-DEXPORT_IntegrationBusAPI)

# Globally add -fPIC compiler option
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

## Allow debug libraries to be installed side-by-side with their release variant
set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Make sure debug binaries differ from release binaries by a suffix 'd'" FORCE)

################################################################################
# Install definitions
################################################################################

include(IntegrationBusInstall)

################################################################################
# Include thirdparty libraries to be built
################################################################################

################################################################################
# Unit testing
################################################################################

# Google mock
add_definitions(-DUNIT_TEST)

if(MSVC)
    # Ensure that std::tuple is used instead of std::tr1::tuple
    add_definitions(
        -DGTEST_LANG_CXX11=1
        -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
    )

    # Prevent overriding the parent project's compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif(MSVC)

# ================================================================================
#  WARNING: google test must be added before Fast-CDR / Fast-RTPS
#
#  Otherwise, Fast-CDR or Fast-RTPS will set some CMake Variables that make
#  linking on the google test cases fail with Visual Studio.
# ================================================================================
function(addTestProject)
    # HACK: Temporarily remove the project version to avoid a warning related to unversioned project definitions
    set(PROJECT_VERSION)
    set(PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MINOR)
    set(PROJECT_VERSION_PATCH)

    message(STATUS "Add subdirectory ThirdParty/googletest-1.8.0")
    add_subdirectory(${THIRDPARTY_DIR}/googletest-1.8.0
        ThirdParty/googletest-1.8.0
        EXCLUDE_FROM_ALL
    )
endfunction()
addTestProject()

################################################################################
# Middleware
################################################################################
set(IB_BIN_FASTRTPS_VERSION 1.8.1)
set(IB_BIN_FASTRTPS_REPOSITORY ) #default: must be user set!
option(IB_BIN_FASTRTPS_ENABLE
        "Fetch pre-built and packaged FastRTPS/fastcdr binaries "
        " (set IB_BIN_FASTRTPS_REPOSITORY for download URL)"
)

if(IB_BIN_FASTRTPS_ENABLE)
    include(get_compiler_arch)
    include(GetExternalProject)
    get_compiler_arch(_comp _arch _plat)
    if(${_plat} MATCHES "Linux" )
        get_ubuntu_version(ubuntu)
        set(_plat "ubuntu${ubuntu}")
    endif()
    set(binname "fastrtps-${IB_BIN_FASTRTPS_VERSION}-${_plat}-${_comp}")
    if(NOT IB_BIN_FASTRTPS_REPOSITORY)
        message(FATAL_ERROR 
            "You have to set IB_BIN_FASTRTPS_REPOSITORY to an URL serving"
            " the fastrtps zip file named \"${binname}-$<CONFIG>.zip\"."
            " To override the version set IB_BIN_FASTRTPS_VERSION."
            )
    endif()
    message(STATUS "NOTE: using pre-build fastrtps/fastcdr")
    set(FASTRTPS_BIN_DIR ${CMAKE_BINARY_DIR}/fastrtps-${IB_BIN_FASTRTPS_VERSION})
    get_external_project(fastrtps-release
        URL ${IB_BIN_FASTRTPS_REPOSITORY}/${binname}-Release.zip
        WORKING_DIR ${CMAKE_BINARY_DIR}/fastrtps-download/work-Release
        SOURCE_DIR  ${CMAKE_BINARY_DIR}/fastrtps-download/Release #will be extracted here, deletes existing
        INSTALL_COMMAND   
            "${CMAKE_COMMAND}" -E copy_directory 
            ${CMAKE_BINARY_DIR}/fastrtps-download/Release ${FASTRTPS_BIN_DIR}
        )
    #TODO we need to bundle Debug and Release, but they are mostly the same
    #TODO: we should only copy the cmake exports and the library files ?
    get_external_project(fastrtps-debug
        URL ${IB_BIN_FASTRTPS_REPOSITORY}/${binname}-Debug.zip
        WORKING_DIR ${CMAKE_BINARY_DIR}/fastrtps-download/work-Debug
        SOURCE_DIR  ${CMAKE_BINARY_DIR}/fastrtps-download/Debug 
        INSTALL_COMMAND
            "${CMAKE_COMMAND}" -E copy_directory 
            ${CMAKE_BINARY_DIR}/fastrtps-download/Debug ${FASTRTPS_BIN_DIR}
        )
    #NB the fastcdr-config.cmake explicitly checks for fastcdr_BIN_DIR
    #   but we have static builds pre-compiled with no */bin folder
    set(fastcdr_BIN_DIR ${FASTRTPS_BIN_DIR}/bin 
            CACHE STRING "override vendor package config logic" FORCE)
    set(fastrtps_BIN_DIR ${fastcdr_BIN_DIR}
            CACHE STRING "override vendor package config logic" FORCE)
    file(MAKE_DIRECTORY ${fastrtps_BIN_DIR})
    find_package(fastcdr CONFIG
        REQUIRED
        PATHS ${FASTRTPS_BIN_DIR}
        )
    find_package(fastrtps CONFIG
        REQUIRED
        PATHS ${FASTRTPS_BIN_DIR}
        )
    message(STATUS "bundling fastrtps-${IB_BIN_FASTRTPS_VERSION} from ${FASTRTPS_BIN_DIR}")
	#will install to ./bin, ./lib, ./include etc.
    install(DIRECTORY
            ${FASTRTPS_BIN_DIR}/
            DESTINATION
            ${INSTALL_TOP_DIR}
     )
else()
    ### build from submodule or fetch sources from external mirror or use pre-built binaries (deployed mode)
    option(THIRDPARTY_fastcdr "Activate the use of Fast-RTPS internal thirdparty fastcdr" ON)
    option(THIRDPARTY_Asio "Activate the use of Fast-RTPS internal thirdparty Asio" ON)
    option(TINYXML2_FROM_SOURCE "Integrate TinyXML2 source code inside Fast RTPS" ON)
    option(THIRDPARTY_TinyXML2 "Activate the use of Fast-RTPS internal thirdparty TinyXML2" ON)
    ##################
    #local source tree 
    ##################
    if(EXISTS ${THIRDPARTY_DIR}/Fast-RTPS/CMakeLists.txt)
        function(include_fastrtps_with_install_prefix)
            message(STATUS "Building Fast-RTPS from local submodule")
            message(STATUS "Add subdirectory ThirdParty/Fast-RTPS")
            set(BIN_INSTALL_DIR "${INSTALL_BIN_DIR}")
            set(INCLUDE_INSTALL_DIR "${INSTALL_INCLUDE_DIR}")
            set(LIB_INSTALL_DIR "${INSTALL_LIB_DIR}")
            set(DATA_INSTALL_DIR "${INSTALL_DATA_DIR}")
            set(LICENSE_INSTALL_DIR "${INSTALL_DATA_DIR}/Fast-RTPS")
            add_subdirectory(${THIRDPARTY_DIR}/Fast-RTPS
                ThirdParty/Fast-RTPS
            )
        endfunction()
        include_fastrtps_with_install_prefix()
    ##################
    # We are part of a VIB distribution bundle -- use deployed FastRTPS binaries
    ##################
    elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../../${INSTALL_LIB_DIR}/)
        # NB the current FastRTPS 1.8.x binaries deploy their cmake config into the share instead of lib folder
        set(_release_bin_fastrtps "${CMAKE_CURRENT_LIST_DIR}/../../${INSTALL_TOP_DIR}/share")
        message(STATUS "------------------------------------------------------------------")
        message(STATUS "Using pre-built binary FastRTPS package from ${_release_bin_fastrtps}")
        message(STATUS "------------------------------------------------------------------")
        find_package(fastcdr CONFIG
            REQUIRED
            PATHS ${_release_bin_fastrtps}
            )
        find_package(fastrtps CONFIG
            REQUIRED
            PATHS ${_release_bin_fastrtps}
            )
        if(NOT fastrtps_FOUND)
            message(FATAL_ERROR
                "Can't load the pre-built fastrtps binaries from \"${_release_bin_fastrtps}\""
                " and neither build Fast-RTPS from source via \"${THIRDPARTY_DIR}/Fast-RTPS/CMakeLists.txt!\"")
        endif()
    ##################
    # Use git to fetch the Fast-RTPS source tree
    ##################
    else()
        message(STATUS "------------------------------------------------------------------")
        message(STATUS "          Fetching missing Fast-RTPS sources via git")
        message(STATUS "          This make take several minutes...")
        message(STATUS "------------------------------------------------------------------")
        set(FASTRTPS_URL "https://github.com/eProsima/Fast-RTPS")
        set(FASTRTPS_TAG "v${IB_BIN_FASTRTPS_VERSION}")

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
                ThirdParty/Fast-RTPS
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
                    ThirdParty/Fast-RTPS
                    EXCLUDE_FROM_ALL
                )
            endif()
        endif()
    endif()
endif(IB_BIN_FASTRTPS_ENABLE)

# NB: we use the ASIO shipped with FastRTPS right now
# the binary distribution lacks the Asio sources
# but it should be included/deployed by our packaging tools
set(ASIO_INCLUDE_DIR ${THIRDPARTY_DIR}/Fast-RTPS/thirdparty/asio/asio/include)
if(NOT EXISTS ${ASIO_INCLUDE_DIR})
    message(WARNING "Asio include directory not found: ${ASIO_INCLUDE_DIR}")
else()
    message(STATUS "Asio include directory found:${ASIO_INCLUDE_DIR}")
endif()

# Test related tools
include(IntegrationBusTest)
include(CTest)

################################################################################
# Include of our repositories
################################################################################

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET fastcdr PROPERTY FOLDER "ThirdParty")
set_property(TARGET fastrtps PROPERTY FOLDER "ThirdParty")
set_property(TARGET gmock_main PROPERTY FOLDER "ThirdParty")
set_property(TARGET gtest PROPERTY FOLDER "ThirdParty")

add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(IntegrationTests)

set(_DEPLOY_CMAKE_DIR  ${INSTALL_LIB_DIR}/cmake/IntegrationBus/)

# export our library version for usage in find_package() by a user of the library
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/IntegrationBusConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/IntegrationBusConfigVersion.cmake
    DESTINATION ${_DEPLOY_CMAKE_DIR}
    )
#create a portable packageConfig.cmake
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/IntegrationBusConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/IntegrationBusConfig.cmake
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
    INSTALL_DESTINATION ${_DEPLOY_CMAKE_DIR}
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/IntegrationBusConfig.cmake
    DESTINATION ${_DEPLOY_CMAKE_DIR}
    )


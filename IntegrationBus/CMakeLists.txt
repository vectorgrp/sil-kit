cmake_minimum_required(VERSION 3.5)

################################################################################
# Project definition
################################################################################

project("IntegrationBus-Library" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)

include(${PROJECT_SOURCE_DIR}/../cmake/IntegrationBusVersion.cmake)
configure_ib_version(${PROJECT_NAME})

set(IB_PROJECT_DIR ${PROJECT_SOURCE_DIR})
set(THIRDPARTY_DIR ${PROJECT_SOURCE_DIR}/../ThirdParty)

option(IB_INSTALL_PDB_FILES "Install Visual Studio debug symbol files along with the IB dll" OFF)

if(MSVC)
    add_compile_options("/MP;/W4;/w44062")
endif (MSVC)

add_definitions(-DEXPORT_IntegrationBusAPI)

# Globally add -fPIC compiler option
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

## Allow Linux debug libraries to be installed side-by-side with their release variant
#if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
#    set(CMAKE_DEBUG_POSTFIX d)
#endif()

################################################################################
# Install definitions
################################################################################

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/IntegrationBusInstall.cmake)
configure_ib_install(${PROJECT_NAME})

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
    install(DIRECTORY
            ${FASTRTPS_BIN_DIR}
            DESTINATION
            ThirdParty/bin
     )
else()
    ### build from submodule or fetch sources from external mirror or use pre-built binaries (deployed mode)
    option(THIRDPARTY_fastcdr "Activate the use of Fast-RTPS internal thirdparty fastcdr" ON)
    option(THIRDPARTY_Asio "Activate the use of Fast-RTPS internal thirdparty Asio" ON)
    option(TINYXML2_FROM_SOURCE "Integrate TinyXML2 source code inside Fast RTPS" ON)
    option(THIRDPARTY_TinyXML2 "Activate the use of Fast-RTPS internal thirdparty TinyXML2" ON)
    if(EXISTS ${THIRDPARTY_DIR}/Fast-RTPS/CMakeLists.txt)
        #local source tree -- we export this as binary packages
        message(STATUS "Building Fast-RTPS from local submodule")
        add_subdirectory(${THIRDPARTY_DIR}/Fast-RTPS
            ThirdParty/Fast-RTPS
            EXCLUDE_FROM_ALL
        )
    elseif(TARGET fastcdr AND TARGET fastrtps)
        # the binary targets have already been defined in the deployed top-level CMakeLists
        message(STATUS "fastcdr and fastrtps targets are already defined")
    else()
        message("--------------------------------------------------------------------")
        message("--           Fetching missing Fast-RTPS sources via git")
        message("--")
        message("--           This make take several minutes...")
        message("--------------------------------------------------------------------")
        set(FASTRTPS_URL "https://github.com/eProsima/Fast-RTPS")
        set(FASTRTPS_TAG "release/${IB_BIN_FASTRTPS_VERSION}")

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

################################################################################
# Logging
################################################################################

# Temporarily patch some CMake variables to trick spdlog's install commands into using our custom folder structure
set(CMAKE_INSTALL_LIBDIR_backup "${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_INSTALL_INCLUDEDIR_backup "${CMAKE_INSTALL_INCLUDEDIR}")
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    # Files ./lib/cmake/spdlog/spdlog*.cmake should rather go to ./<Config>/share/cmake/spdlog/spdlog*.cmake
    # File ./pkgconfig/spdlog.pc should rather go to ./<Config>/share/pkgconfig/spdlog.pc
    set(CMAKE_INSTALL_LIBDIR ${INSTALL_CONFIG_DIR}/../..)
    # Folder ./include/spdlog/**/*.h should rather go to ./<Config>/include/spdlog/**/*.h on Windows
    set(CMAKE_INSTALL_INCLUDEDIR ${INSTALL_INCLUDE_DIR})
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    # Files ./lib/cmake/spdlog/spdlog*.cmake should rather go to ./<System>/cmake/spdlog/spdlog*.cmake
    # File ./pkgconfig/spdlog.pc should rather go to ./<System>/pkgconfig/spdlog.pc
    set(CMAKE_INSTALL_LIBDIR ${SYSTEM_TAG})
    # Folder ./include/spdlog/**/*.h should rather go to ./<System>/include/spdlog/**/*.h on Windows
    set(CMAKE_INSTALL_INCLUDEDIR ${INSTALL_INCLUDE_DIR})
endif()

add_subdirectory(${THIRDPARTY_DIR}/spdlog
    ThirdParty/spdlog
)

# Restore previously patched CMake variables
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR_backup}")
set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR_backup}")

################################################################################
# Helper Functions
################################################################################

function(add_vib_test)
    if(NOT ${IB_BUILD_TESTS})
        return()
    endif()

    set(multiValueArgs SOURCES LIBS CONFIGS)

    cmake_parse_arguments(PARSED_ARGS
        ""
        ""
        "${multiValueArgs}"
        ${ARGN}
    )

    if(NOT PARSED_ARGS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "add_ib_test function failed because no executable name was specified (UNPARSED_ARGUMENTS were empty).")
    endif()

    list(GET PARSED_ARGS_UNPARSED_ARGUMENTS 0 executableName)

    if(NOT PARSED_ARGS_SOURCES)
        message(FATAL_ERROR "add_ib_test function for ${executableName} has an empty source list.")
    endif()

    add_executable(${executableName}
        ${PARSED_ARGS_SOURCES}
    )

    set_property(TARGET ${executableName} PROPERTY FOLDER "Tests")

    target_link_libraries(${executableName}
        PRIVATE IbInterface
        gtest
        gmock_main
        ${PARSED_ARGS_LIBS}
    )

    foreach(config ${PARSED_ARGS_CONFIGS})
        get_filename_component(configFileName ${config} NAME)
        add_custom_command(
            TARGET ${executableName} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/${config} $<TARGET_FILE_DIR:${executableName}>/${configFileName}
        )
    endforeach()

    if (MSVC)
        target_compile_options(${executableName} PRIVATE "/bigobj")
    else()
        target_compile_options(${executableName} PUBLIC "-Wno-inconsistent-missing-override")
    endif(MSVC)

    add_test(NAME ${executableName}
             COMMAND ${executableName} --gtest_output=xml:${executableName}_gtestresults.xml
             WORKING_DIRECTORY $<TARGET_FILE_DIR:${executableName}>
    )
endfunction(add_vib_test)

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

################################################################################
# IDE settings
################################################################################

message(STATUS "Copying VisualStudio .editorconfig into ${CMAKE_CURRENT_BINARY_DIR}")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/.editorconfig DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#configure_file(.editorconfig .editorconfig COPYONLY)

################################################################################
# Install targets for the IntegrationBus
################################################################################

# Save the IntegrationBusTargets into file IntegrationBusTargets.cmake
install(EXPORT IntegrationBusTargets
    # Without overriding the file name, these default file names will be exported:
    #FILE IntegrationBusTargets.cmake
    #FILE IntegrationBusTargets-${CONFIG_TAG}.cmake
    DESTINATION ${INSTALL_CONFIG_DIR}
    COMPONENT IntegrationBus
)

# This is enough to use the IntegrationBus::IntegrationBus target from external projects.
# Register the target so the find_package command can locate it
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/IntegrationBusConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/IntegrationBusConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/IntegrationBusConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIG_DIR}
)

# Install sources
# Copy all files from the source directory to the proper destination
# Filter by file types in case someone spoiled this source folder by calling "cmake ."
install(
    DIRECTORY 
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}/IntegrationTests
    DESTINATION ${INSTALL_SOURCE_DIR}
    COMPONENT IntegrationBus-Developer
    FILES_MATCHING 
        PATTERN *.cpp 
        PATTERN *.cxx 
        PATTERN *.rc 
        PATTERN *.h 
        PATTERN *.hpp 
        PATTERN *.hpp.in 
        PATTERN *.json 
        PATTERN *.cmake 
        PATTERN *.cmake.in 
        PATTERN CMakeLists.txt 
        PATTERN *.md
		PATTERN *.xml
)
install(
    FILES 
        .editorconfig
        CHANGELOG.md
        CMakeLists.txt 
        LICENSE 
    DESTINATION ${INSTALL_SOURCE_DIR}
    COMPONENT IntegrationBus-Developer
)
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../cmake
    DESTINATION ${INSTALL_SOURCE_DIR}/../
    COMPONENT IntegrationBus-Developer
    FILES_MATCHING 
        PATTERN *.cmake
)
# Full installation of checked in third party libraries
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty
    DESTINATION ${INSTALL_SOURCE_DIR}/../
    COMPONENT IntegrationBus-Developer
    # Rational: Fast-RTPS cannot be delivered from source, as it requires its own submodules to be actual Git repositories.
    REGEX "/ThirdParty/Fast-RTPS" EXCLUDE
    # Breathe is just required for generating the documentation
    REGEX "/ThirdParty/Breathe" EXCLUDE
)

# Install all the helper files
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/IntegrationBusConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/IntegrationBusConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIG_DIR}
    COMPONENT IntegrationBus
)

# Install supplementary files
install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
        ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md
    DESTINATION ${INSTALL_DOCDIR}
    COMPONENT IntegrationBus
)

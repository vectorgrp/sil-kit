cmake_minimum_required(VERSION 3.5)

#######################################################################################################################
# Project definition
#######################################################################################################################

project("IntegrationBus-Demos" LANGUAGES CXX C)

# Versioning disabled in a self-contained deployment
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/IntegrationBusVersion.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/IntegrationBusVersion.cmake)
    configure_ib_version(${PROJECT_NAME})
endif()

set(CMAKE_CXX_STANDARD 14)

#######################################################################################################################
# Install definitions
#######################################################################################################################

# Installation unsupported in a self-contained deployment
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/IntegrationBusInstall.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/IntegrationBusInstall.cmake)
    configure_ib_install(${PROJECT_NAME})
    set(ib_can_install TRUE CACHE BOOL "")
endif()

#######################################################################################################################
# Dependencies
#######################################################################################################################

# Try to resolve from installed packages only if not defined previously via add_library
if(NOT TARGET IntegrationBus)
    foreach(path ${CMAKE_PREFIX_PATH})
        message(STATUS "Searching for IntegrationBus library at prefix path: " ${path})
    endforeach(path)
    message(STATUS "Note: Set '-DCMAKE_PREFIX_PATH=<path>' so that ./cmake/IntegrationBusTargets.cmake resolves.")
    find_package(IntegrationBus ${PROJECT_VERSION} REQUIRED)
else()
    message(STATUS "Linking to IntegrationBus library that is built in scope of root project")
endif()

#######################################################################################################################
# Helper Functions
#######################################################################################################################

set(make_ib_demo_caller_dir ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

# Generate IB Demo Project
function(make_ib_demo executableName demoSourceFile)
    add_executable(${executableName}
        ${demoSourceFile}
    )
    remove_definitions(-DEXPORT_IntegrationBusAPI)
    
    # Group this demo project into a folder
    set_property(TARGET ${executableName} PROPERTY FOLDER "Demos")

    target_link_libraries(${executableName}
        PRIVATE IntegrationBus
    )

    if(ib_can_install)
        install(
            TARGETS ${executableName}
            #LIBRARY DESTINATION ${INSTALL_LIBDIR} # Destination for shared libraries
            #ARCHIVE DESTINATION ${INSTALL_LIBDIR} # Destination for static libraries
            RUNTIME DESTINATION ${INSTALL_BINDIR} # Destination for executables and dynamic libraries 
            #PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDEDIR} # Destination for header files
            COMPONENT IntegrationBus-Demos
        )

        get_filename_component(project_dir ${CMAKE_CURRENT_SOURCE_DIR} NAME)

        # Install supplementary files required to use the project executable (e.g., launcher config files)
        install(
            DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DESTINATION ${INSTALL_CODE_DIR}
            COMPONENT IntegrationBus-Demos
            FILES_MATCHING 
                PATTERN *.json 
        )

        # Install project sources
        # Copy all files from the source directory to the proper destination
        # Filter by file types in case someone spoiled this source folder by calling "cmake ."
        install(
            DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DESTINATION ${INSTALL_SOURCE_DIR}
            COMPONENT IntegrationBus-Developer
            FILES_MATCHING 
                PATTERN *.cpp 
                PATTERN *.hpp 
                PATTERN *.json 
        )
        install(
            FILES
                ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
            DESTINATION ${INSTALL_SOURCE_DIR}/${project_dir}
            COMPONENT IntegrationBus-Developer
        )

        # Install project sources for demo developers (CMake + self-contained build scripts)
        install(
            DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DESTINATION ${INSTALL_CODE_DIR}
            COMPONENT IntegrationBus-Demos-Developer
            FILES_MATCHING 
                PATTERN *.cpp 
                PATTERN *.hpp 
                PATTERN *.txt 
                PATTERN *.idl
                PATTERN *.xml
        )
        install(
            DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../cmake
            DESTINATION ${INSTALL_CODE_DIR}/../
            COMPONENT IntegrationBus-Demos-Developer
            FILES_MATCHING 
                PATTERN *.cmake 
        )
        # Install VisualStudio projects or build scripts
        if(MSVC)
            get_filename_component(PROJECT_SOURCE_NAME ${demoSourceFile} NAME_WE)
            set(PROJECT_EXECUTABLE_NAME "${executableName}")
            # Enable configurations 'Debug|Win32' and 'Release|Win32', or 'Debug|x64' and 'Release|x64'
            if ("${SYSTEM_TAG}" STREQUAL "Win64")
                set(PROJECT_PLATFORM_TAG "x64")  # VisualStudio .vcxproj would not recognize 'Win64' as target platform
            elseif ("${SYSTEM_TAG}" STREQUAL "Win32")
                set(PROJECT_PLATFORM_TAG "Win32")
            else ()
                message(FATAL_ERROR "Bitness is not supported")
            endif ()
            configure_file(
                ${make_ib_demo_caller_dir}/IntegrationBus-Demos_vs140.vcxproj.in
                ${CMAKE_CURRENT_BINARY_DIR}/${executableName}_vs140.vcxproj
            )
            install(
                FILES ${CMAKE_CURRENT_BINARY_DIR}/${executableName}_vs140.vcxproj
                DESTINATION ${INSTALL_CODE_DIR}/${project_dir}
                COMPONENT IntegrationBus-Demos-Developer
            )
        else()
            get_filename_component(PROJECT_SOURCE_NAME ${demoSourceFile} NAME_WE)
            set(PROJECT_EXECUTABLE_NAME "${executableName}")
            configure_file(
                ${make_ib_demo_caller_dir}/IntegrationBus-Demos_gcc.makefile.in
                ${CMAKE_CURRENT_BINARY_DIR}/${executableName}_gcc.makefile
            )
            install(
                FILES ${CMAKE_CURRENT_BINARY_DIR}/${executableName}_gcc.makefile
                DESTINATION ${INSTALL_CODE_DIR}/${project_dir}
                COMPONENT IntegrationBus-Demos-Developer
            )
        endif()
    endif()
endfunction(make_ib_demo)

#######################################################################################################################
# Include of our repositories
#######################################################################################################################

add_subdirectory(Can)
add_subdirectory(Ethernet)
add_subdirectory(FlexRay)
add_subdirectory(Lin)
add_subdirectory(Io)
add_subdirectory(GenericMessage)
add_subdirectory(TickTickDone)
add_subdirectory(ConfigBuilder)
add_subdirectory(SystemMonitor)
add_subdirectory(SystemController)

#######################################################################################################################
# VisualStudio specific setup
#######################################################################################################################

if(MSVC)
    # Deploy VisualStudio configuration file
    message(STATUS "Copying VisualStudio .editorconfig into ${CMAKE_CURRENT_BINARY_DIR}")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/.editorconfig DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    # Create and deploy VisualStudio debug information files for all targets
    add_compile_options("/Zi")
endif(MSVC)

#######################################################################################################################
# Install targets
#######################################################################################################################

if(ib_can_install)
    # Install targets for particular components, so one can simply 'cmake --build . --target install_<Name>'
    # https://stackoverflow.com/questions/9190098/for-cmakes-install-command-what-can-the-component-argument-do
    add_custom_target(install_IntegrationBus-Demos
    COMMAND "${CMAKE_COMMAND}" -DCOMPONENT=IntegrationBus-Demos -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    COMMENT "Installing component 'IntegrationBus-Demos'"
    VERBATIM
    )
    add_custom_target(install_IntegrationBus-Demos-Developer
    COMMAND "${CMAKE_COMMAND}" -DCOMPONENT=IntegrationBus-Demos-Developer -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    COMMENT "Installing component 'IntegrationBus-Demos-Developer'"
    VERBATIM
    )

    # Install sources
    # Copy all files from the source directory to the proper destination
    install(
        FILES 
            .editorconfig
            CMakeLists.txt 
            IntegrationBus-Demos_vs140.sln
            IntegrationBus-Demos_vs140.vcxproj.in
            IntegrationBus-Demos_gcc.makefile.in
            LICENSE 
            README.md
            Makefile
        DESTINATION ${INSTALL_SOURCE_DIR}
        COMPONENT IntegrationBus-Developer
    )

    # Install CMakeFile, VisualStudio solution or build script for demo developers
    install(
        FILES
            .editorconfig
            CMakeLists.txt
        DESTINATION ${INSTALL_CODE_DIR}
        COMPONENT IntegrationBus-Demos-Developer
    )
    if(MSVC)
        install(
            FILES
                IntegrationBus-Demos_vs140.sln
                IntegrationBus-Demos_vs140.vcxproj.in
            DESTINATION ${INSTALL_CODE_DIR}
            COMPONENT IntegrationBus-Demos-Developer
    )
    else()
        install(
            FILES
                Makefile
                IntegrationBus-Demos_gcc.makefile.in
            DESTINATION ${INSTALL_CODE_DIR}
            COMPONENT IntegrationBus-Demos-Developer
    )
    endif()

    # Install supplementary files
    install(
        FILES
            LICENSE
            README.md
        DESTINATION ${INSTALL_CODE_DIR}
        COMPONENT IntegrationBus-Demos
    )
endif()

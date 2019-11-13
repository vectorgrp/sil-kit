cmake_minimum_required(VERSION 3.5)

#######################################################################################################################
# Project definition
#######################################################################################################################
project("IntegrationBus-Demos" LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)

set(_is_standalone FALSE)
if(NOT TARGET IntegrationBus)
    #assuming we are standalone but still within the original distribution directory layout
    #so the source release should be located in ../${INSTALL_SOURCE_DIR}
    message(STATUS "<-> no IntegrationBus-Library target found. working in distribution mode")
    message(STATUS "<-> using deployed IntegrationBus")
    #XXX kludge to make fmt dependencies of exported target work
    include(${CMAKE_CURRENT_LIST_DIR}/../IntegrationBus-Source/ThirdParty/fmt.cmake)
    find_package(IntegrationBus
        REQUIRED
        CONFIG
        NO_CMAKE_PACKAGE_REGISTRY
        NO_DEFAULT_PATH
        PATHS "${CMAKE_CURRENT_LIST_DIR}/../IntegrationBus"
    )
    set(_is_standalone TRUE)
else()
    include(IntegrationBusInstall)
    include(IntegrationBusVersion)
    configure_ib_version(${PROJECT_NAME})
endif()


#######################################################################################################################
# Dependencies
#######################################################################################################################

# Try to resolve from installed packages only if not defined previously via add_library
if(NOT TARGET IntegrationBus::IntegrationBus)
    foreach(path ${CMAKE_PREFIX_PATH})
        message(STATUS "Searching for IntegrationBus library at prefix path: " ${path})
    endforeach(path)
    message(STATUS "IB-Demos: Note: Set '-DCMAKE_PREFIX_PATH=<path>' so that ./cmake/IntegrationBusTargets.cmake resolves.")
    find_package(IntegrationBus ${PROJECT_VERSION} REQUIRED)
else()
    message(STATUS "Linking to IntegrationBus library that is built in scope of root project")
endif()

#######################################################################################################################
# Helper Functions
#######################################################################################################################

set(make_ib_demo_caller_dir ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

set(demo_list)
add_custom_target(Demos COMMENT "Build all available Demos")
# Generate IB Demo Project
function(make_ib_demo executableName demoSourceFile)
    add_executable(${executableName}
        EXCLUDE_FROM_ALL
        ${demoSourceFile}
    )
    if(_is_standalone)
        set_target_properties(${executableName} PROPERTIES EXCLUDE_FROM_ALL FALSE)
        set(_DEMO_OUTPUT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../IntegrationBus/bin")
    else()
        set(_DEMO_OUTPUT_DIR "${CMAKE_BINARY_DIR}/Demos/$<CONFIG>/bin")
    endif()
    remove_definitions(-DEXPORT_IntegrationBusAPI)
    
    # Group this demo project into a folder
    set_target_properties(${executableName} PROPERTIES
        FOLDER "Demos"
        RUNTIME_OUTPUT_DIRECTORY "${_DEMO_OUTPUT_DIR}"
        )

    target_link_libraries(${executableName}
        PRIVATE IntegrationBus::IntegrationBus
    )
    add_dependencies(Demos ${executableName})
    set(demo_list "${demo_list} ${executableName}" PARENT_SCOPE)
endfunction(make_ib_demo)

#######################################################################################################################
# Add the actual demo projects
#######################################################################################################################
add_subdirectory(Can)
add_subdirectory(Ethernet)
add_subdirectory(FlexRay)
add_subdirectory(Lin)
add_subdirectory(Io)
add_subdirectory(GenericMessage)
add_subdirectory(TickTickDone)
add_subdirectory(ConfigBuilder)
add_subdirectory(SystemControllerInteractive)
add_subdirectory(GenericMessageITest)

message(STATUS "Demos available: ${demo_list}")
#######################################################################################################################
# VisualStudio specific setup
#######################################################################################################################

if(MSVC)
    # Create and deploy VisualStudio debug information files for all targets
    add_compile_options("/Zi")
endif(MSVC)

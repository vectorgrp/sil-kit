cmake_minimum_required(VERSION 3.5)

#######################################################################################################################
# Project definition
#######################################################################################################################

project("IntegrationBus-Library" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)

include(${PROJECT_SOURCE_DIR}/../cmake/IntegrationBusVersion.cmake)
configure_ib_version(${PROJECT_NAME})

set(IB_PROJECT_DIR ${PROJECT_SOURCE_DIR})
set(THIRDPARTY_DIR ${PROJECT_SOURCE_DIR}/../ThirdParty)

option(IB_INSTALL_PDB_FILES "Enable creating multi-config packages from multiple build folders" OFF)

if(MSVC)
    add_compile_options("/MP")
endif (MSVC)

add_definitions(-DEXPORT_IntegrationBusAPI)

# Globally add -fPIC compiler option
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

## Allow Linux debug libraries to be installed side-by-side with their release variant
#if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
#    set(CMAKE_DEBUG_POSTFIX d)
#endif()

#######################################################################################################################
# Install definitions
#######################################################################################################################

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/IntegrationBusInstall.cmake)
configure_ib_install(${PROJECT_NAME})

#######################################################################################################################
# Include thirdparty libraries to be built
#######################################################################################################################

#######################################################################################################################
# Unit testing
#######################################################################################################################

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

#######################################################################################################################
# Middleware
#######################################################################################################################

#add_subdirectory(${THIRDPARTY_DIR}/Fast-RTPS/thirdparty/fastcdr
#    ThirdParty/fastcdr
#    EXCLUDE_FROM_ALL
#)
option(THIRDPARTY_fastcdr "Activate the use of Fast-RTPS internal thirdparty fastcdr" ON)
option(THIRDPARTY_Asio "Activate the use of Fast-RTPS internal thirdparty Asio" ON)
option(TINYXML2_FROM_SOURCE "Integrate TinyXML2 source code inside Fast RTPS" ON)
option(THIRDPARTY_TinyXML2 "Activate the use of Fast-RTPS internal thirdparty TinyXML2" ON)

add_subdirectory(${THIRDPARTY_DIR}/Fast-RTPS
    ThirdParty/Fast-RTPS
    EXCLUDE_FROM_ALL
)

#######################################################################################################################
# Helper Functions
#######################################################################################################################

# Add IntegrationBus Headers to VS Projects
function(add_mvsc_ib_include_filter)
    if(CMAKE_VERSION VERSION_GREATER 3.8.0)
        # Easy access to IntegrationBus includes for Visual Studio
        file(GLOB_RECURSE IB_INCLUDES_ALL ${IB_PROJECT_DIR}/include/ib/*.hpp)
        source_group(TREE ${IB_PROJECT_DIR}/include/ib PREFIX "Header Files (IB)" FILES ${IB_INCLUDES_ALL})
    endif()
endfunction(add_mvsc_ib_include_filter)

# Generate Gtest Project
function(make_gtest executableName testSourceFile)
    add_executable(${executableName}
        ${testSourceFile}
    )
    
    set_property(TARGET ${executableName} PROPERTY FOLDER "Tests")

    target_link_libraries(${executableName}
        PRIVATE IbInterface
        gtest
        gmock_main
        ${ARGV2}
        ${ARGV3}
    )

    if(NOT MSVC)
        target_compile_options(${executableName} PUBLIC "-Wno-inconsistent-missing-override")
    endif()

    add_test(${executableName} ${executableName} --gtest_output=xml:${executableName}_gtestresults.xml)
  
    add_mvsc_ib_include_filter()
endfunction(make_gtest)

set(make_integration_test_caller_dir ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

function(make_integration_test testName)
    set(executableName ITest${testName})
    
    add_executable(${executableName}
        ${testName}ITest.cpp
        )

    set_property(TARGET ${executableName} PROPERTY FOLDER "Tests")

    target_link_libraries(${executableName}
        PRIVATE IbInterface
        PRIVATE IntegrationBus
        gtest
        gmock_main
        )

    if(NOT MSVC)
        target_compile_options(${executableName} PUBLIC "-Wno-inconsistent-missing-override")
    endif()

    add_test(
        NAME ${executableName}
        COMMAND ${CMAKE_COMMAND} -DPATH=$<TARGET_FILE_DIR:IntegrationBus>\;$<TARGET_FILE_DIR:fastrtps>\;$<TARGET_FILE_DIR:fastcdr> -P ${make_integration_test_caller_dir}/cmake/GTestRunner.cmake $<TARGET_FILE:${executableName}>
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    add_mvsc_ib_include_filter()
endfunction(make_integration_test executableName)

include(CTest)

#######################################################################################################################
# Include of our repositories
#######################################################################################################################

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET fastcdr PROPERTY FOLDER "ThirdParty")
set_property(TARGET fastrtps PROPERTY FOLDER "ThirdParty")
set_property(TARGET gmock_main PROPERTY FOLDER "ThirdParty")
set_property(TARGET gtest PROPERTY FOLDER "ThirdParty")

add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(IntegrationTests)

#######################################################################################################################
# Middleware
#######################################################################################################################

message(STATUS "Copying VisualStudio .editorconfig into ${CMAKE_CURRENT_BINARY_DIR}")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/.editorconfig DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#configure_file(.editorconfig .editorconfig COPYONLY)

#######################################################################################################################
# Install targets for the IntegrationBus
#######################################################################################################################

# Install targets for particular components, so one can simply 'cmake --build . --target install_<Name>'
# https://stackoverflow.com/questions/9190098/for-cmakes-install-command-what-can-the-component-argument-do
add_custom_target(install_IntegrationBus
    COMMAND "${CMAKE_COMMAND}" -DCOMPONENT=IntegrationBus -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    COMMENT "Installing component 'IntegrationBus'"
    VERBATIM
)
add_custom_target(install_IntegrationBus-Developer
    COMMAND "${CMAKE_COMMAND}" -DCOMPONENT=IntegrationBus-Developer -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    COMMENT "Installing component 'IntegrationBus-Developer'"
    VERBATIM
)

# Save the IntegrationBusTargets into file IntegrationBusTargets.cmake
install(EXPORT IntegrationBusTargets
    # Without overriding the file name, these default file names will be exported:
    #FILE IntegrationBusTargets.cmake
    #FILE IntegrationBusTargets-${CONFIG_TAG}.cmake
    DESTINATION ${INSTALL_CONFIG_DIR}
    COMPONENT IntegrationBus
)

# Install binaries of dependent libraries
install(
    FILES $<TARGET_FILE:fastrtps>
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT IntegrationBus
)
install(
    FILES $<TARGET_FILE:fastcdr>
    DESTINATION ${INSTALL_LIBDIR}
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
)
install(
    FILES 
        .editorconfig
        CHANGELOG.md
        CMakeLists.txt 
        LICENSE 
        CloneFastRTPS.bat.in 
        CloneFastRTPS.sh.in 
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
)
# Instead of the Fast-RTPS library, deploy a script to clone Fast-RTPS. 
# It should only be used from package deployments, thus the '.in' disguise.
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/CloneFastRTPS.sh.in
        ${CMAKE_CURRENT_BINARY_DIR}/CloneFastRTPS.sh
        COPYONLY
    )
    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/CloneFastRTPS.sh
        DESTINATION ${INSTALL_SOURCE_DIR}/../ThirdParty/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE  # chmod +x
        COMPONENT IntegrationBus-Developer
    )
else()
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/CloneFastRTPS.bat.in
        ${CMAKE_CURRENT_BINARY_DIR}/CloneFastRTPS.bat
        COPYONLY
    )
    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/CloneFastRTPS.bat
        DESTINATION ${INSTALL_SOURCE_DIR}/../ThirdParty/
        COMPONENT IntegrationBus-Developer
    )
endif()

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

#######################################################################################################################
# Configure the IntegrationBus packaging groups: Workaround to assign the same component to multiple component groups.
#######################################################################################################################

# CPack does not allow to add the same component to multiple component groups. 
# This workaround enables the 'all-components-in-one' mode, and enables just the right components
# Notes: 
# * This file is executed at CPack runtime
# * Run 'cpack -D CPACK_COMPONENT_GROUP=<group name>' to generate a group's particular package

# Ignore groups, as they do not allow one component to be shared by multiple group
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

# Instead, define which components to include based on the CPACK_COMPONENT_GROUP if this cpack run
if("${CPACK_COMPONENT_GROUP}" STREQUAL "IntegrationBus-Library-Package")
    set(CPACK_COMPONENTS_ALL 
        IntegrationBus 
        IntegrationBus-Launcher 
        IntegrationBus-Demos
    )
elseif("${CPACK_COMPONENT_GROUP}" STREQUAL "IntegrationBus-Contributor-Package")
    set(CPACK_COMPONENTS_ALL 
        IntegrationBus 
        IntegrationBus-Launcher 
        IntegrationBus-Demos
        IntegrationBus-Developer
        IntegrationBus-Demos-Developer
        Unspecified  # Install commands of third-party libraries like spdlog do not specify a component, they end up here
    )
else()
    message(FATAL_ERROR "Call 'cpack -D CPACK_COMPONENT_GROUP=<group name>' with one of 'IntegrationBus-Library-Package' or 'IntegrationBus-Contributor-Package'")
endif()

# Ensure that all requested build folders are built so packaging can start
list(LENGTH CPACK_MULTICONFIG_BUILDFOLDERS count)
math(EXPR count "${count}-1")
foreach(i RANGE ${count})
    list(GET CPACK_MULTICONFIG_BUILDFOLDERS ${i} buildFolder)

    get_filename_component(buildFolderAbsolute "${buildFolder}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
    if(NOT EXISTS "${buildFolder}/CMakeCache.txt")
       message(FATAL_ERROR "Expected a build at ${buildFolderAbsolute}, but found none")
    endif()
endforeach()

# Reuse the group's previously configured file name (CMake 3.5.x uses CPACK_PACKAGE_FILE_NAME is used)
string(TOUPPER ${CPACK_COMPONENT_GROUP} CPACK_COMPONENT_GROUP_UPPER)
set(CPACK_ARCHIVE_FILE_NAME "${CPACK_ARCHIVE_${CPACK_COMPONENT_GROUP_UPPER}_FILE_NAME}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_ARCHIVE_FILE_NAME}")

message(STATUS "Creating package for CPACK_COMPONENT_GROUP '${CPACK_COMPONENT_GROUP}'")

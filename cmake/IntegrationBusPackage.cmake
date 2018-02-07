#######################################################################################################################
# Configure the IntegrationBus packaging
#######################################################################################################################

# Cf. https://gitlab.kitware.com/cmake/community/wikis/doc/cpack/Component-Install-With-CPack
# Call 'cmake -DIB_SPRINT_NUMBER=<sprint number>' to include the sprint number in the package file names

#######################################################################################################################
# Set common CPACK variables.
#######################################################################################################################

set(CPACK_PACKAGE_NAME "IntegrationBus")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "IntegrationBus bundle")
set(CPACK_PACKAGE_DESCRIPTION "This is a package of the IntegrationBus")

set(CPACK_PACKAGE_VENDOR "Vector Informatik GmbH")
set(CPACK_PACKAGE_CONTACT "IntegrationBus Support <support@vector.com>")

# Canonical versions (The tweak number will be encoded as the development sprint number)
set(CPACK_PACKAGE_VERSION       ${${CPACK_PACKAGE_NAME}_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${${CPACK_PACKAGE_NAME}_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${${CPACK_PACKAGE_NAME}_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${${CPACK_PACKAGE_NAME}_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION_TWEAK ${${CPACK_PACKAGE_NAME}_VERSION_TWEAK})

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/IntegrationBus/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/IntegrationBus/CHANGELOG.md")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/IntegrationBus/CHANGELOG.md")

# Disable component-based packaging in favor of component group-based packaging (for archive generators TGZ, ZIP)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
# Obtain one archive file per component group defined above
set(CPACK_COMPONENT_GROUPS_ALL IntegrationBus-Library-Package IntegrationBus-Contributor-Package)
# List all components above, so that CPack does not generate (empty) archives for additional ThirdParty components
set(CPACK_COMPONENTS_ALL IntegrationBus IntegrationBus-Developer IntegrationBus-Demos IntegrationBus-Demos-Developer IntegrationBus-Launcher)

# Default archive format for source and binary packages depending on target platform, overridable via 'cpack -G <GENERATOR>'
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_SOURCE_GENERATOR "TGZ")
else()
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_SOURCE_GENERATOR "ZIP")
endif()

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF)

#######################################################################################################################
# Components and component groups
#######################################################################################################################

# We must include CPack component macros before any cpack_* commands
include(CPackComponent)

#cpack_add_install_type(Full DISPLAY_NAME "Everything")
#cpack_add_install_type(Developer DISPLAY_NAME "Sources")
#cpack_add_install_type(Demos DISPLAY_NAME "Demo projects")
#cpack_add_install_type(Launcher DISPLAY_NAME "Launcher tool")

cpack_add_component_group(IntegrationBus-Library-Package
    DISPLAY_NAME "IntegrationBus"
    DESCRIPTION "The IntegrationBus library: binaries of library and demo projects, plus headers and demo project build scripts"
)
cpack_add_component_group(IntegrationBus-Contributor-Package
    DISPLAY_NAME "IntegrationBus-Contributor"
    DESCRIPTION "The IntegrationBus for contributors: sources and binaries of library and demo projects"
)

# Packages of the IntegrationBus-Library-Package and also of the IntegrationBus-Contributor-Package
cpack_add_component(IntegrationBus
    #REQUIRED
    DISPLAY_NAME "IntegrationBus"
    DESCRIPTION "IntegrationBus library"
    GROUP IntegrationBus-Library-Package
    #GROUP IntegrationBus-Contributor-Package
    #INSTALL_TYPES Full
)
cpack_add_component(IntegrationBus-Launcher
    DISPLAY_NAME "IntegrationBus Launcher"
    DESCRIPTION "IntegrationBus launcher command-line tool, requires Python"
    GROUP IntegrationBus-Library-Package
    #GROUP IntegrationBus-Contributor-Package
    #DEPENDS IntegrationBus
    #INSTALL_TYPES Full Demos Launcher
)
cpack_add_component(IntegrationBus-Demos
    DISPLAY_NAME "IntegrationBus Demos"
    DESCRIPTION "IntegrationBus demo projects with sources and simple build scripts"
    GROUP IntegrationBus-Library-Package
    #GROUP IntegrationBus-Contributor-Package
    #DEPENDS IntegrationBus
    #INSTALL_TYPES Full Demos
)

# Additional packages of the IntegrationBus-Contributor-Package
cpack_add_component(IntegrationBus-Developer
    DISPLAY_NAME "IntegrationBus Developer"
    DESCRIPTION "IntegrationBus library for library contributors"
    GROUP IntegrationBus-Contributor-Package
    #DEPENDS IntegrationBus
    #INSTALL_TYPES Full Developer
)
cpack_add_component(IntegrationBus-Demos-Developer
    DISPLAY_NAME "IntegrationBus Demos for Developers"
    DESCRIPTION "IntegrationBus demo projects with sources for demo project contributors"
    GROUP IntegrationBus-Contributor-Package
    #DEPENDS IntegrationBus-Developer #IntegrationBus-Launcher
    #INSTALL_TYPES Full Demos Developer
)

#######################################################################################################################
# Platform and architecture dependent packaging
#######################################################################################################################

# Based on the component groups' display name, derive archive names following our own naming scheme
include(${CMAKE_CURRENT_LIST_DIR}/IntegrationBusPackageName.cmake)
configure_ib_package_name()

# Option to group multiple build folders with different target configurations into single packages
include(${CMAKE_CURRENT_LIST_DIR}/IntegrationBusPackageMultiConfig.cmake)
configure_ib_multiconfig_package()

# Adding one component to multiple groups is not supported: http://cmake.3232098.n2.nabble.com/cpack-multiple-packages-td7594772.html
# Our work-around is to use the CPACK_GENERATOR_CONFIG_FILE which can configure the components depending on CPACK_COMPONENT_GROUP at cpack-time:
# $ cpack -D CPACK_COMPONENT_GROUP=<group name>
if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/IntegrationBusPackageConfig.cmake")
    message(FATAL_ERROR "CPack runtime configuration file '${CMAKE_CURRENT_LIST_DIR}/IntegrationBusPackageConfig.cmake' is missing")
endif()
set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_CURRENT_LIST_DIR}/IntegrationBusPackageConfig.cmake")

#######################################################################################################################
# Create the CPackConfig.cmake file
#######################################################################################################################

# This must always be after all CPACK_PACKAGE_* definitions
include(CPack)

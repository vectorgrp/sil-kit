cmake_minimum_required(VERSION 3.5)

#######################################################################################################################
# Project definition
#######################################################################################################################

project("IntegrationBus")

include(${PROJECT_SOURCE_DIR}/cmake/IntegrationBusVersion.cmake)
configure_ib_version(${PROJECT_NAME})

#######################################################################################################################
# Include thirdparty libraries to be built
#######################################################################################################################

######################################################################################################################
# Include the IntegrationBus projects to be built
#######################################################################################################################

# Enable testing for this project
enable_testing()

# Have both IntegrationBus library and demo project in a single solution (for debugging purposes and packaging)
add_subdirectory(${PROJECT_SOURCE_DIR}/IntegrationBus)
add_subdirectory(${PROJECT_SOURCE_DIR}/Demos)
# Include the Python-implemented launcher tool for testing and installation
add_subdirectory(${PROJECT_SOURCE_DIR}/Launcher)

#######################################################################################################################
# Packaging
#######################################################################################################################

include(${PROJECT_SOURCE_DIR}/cmake/IntegrationBusPackage.cmake)

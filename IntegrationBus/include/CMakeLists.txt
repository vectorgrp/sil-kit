file(GLOB_RECURSE IB_INCLUDES_ALL ${IB_PROJECT_DIR}/include *.hpp)

add_library(IbInterface INTERFACE)

# Only add headers as sources for builds, so that IDEs list them when a project depends on this library
# https://cmake.org/pipermail/cmake/2016-July/063907.html
target_sources(IbInterface
    INTERFACE
        $<BUILD_INTERFACE:${IB_INCLUDES_ALL}>
)

# Use generator expression to have a different set of include paths when the target is built versus when the target is installed somewhere
target_include_directories(IbInterface
    INTERFACE
        $<BUILD_INTERFACE:${IB_PROJECT_DIR}/include>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
)

target_link_libraries(IbInterface
    INTERFACE spdlog
)


#######################################################################################################################
# add to install targets of the IntegrationBus
#######################################################################################################################

# Mark the IntegrationBus headers for export by adding it to the target container IntegrationBusTargets
install(
    TARGETS IbInterface
    EXPORT IntegrationBusTargets
    COMPONENT IntegrationBus
    INCLUDES DESTINATION ${INSTALL_INCLUDE_DIR} # Destination for header files
)

# Copy all headers from the source directory to the proper destination
install(
    DIRECTORY ${IB_PROJECT_DIR}/include/
    DESTINATION ${INSTALL_INCLUDE_DIR}
    COMPONENT IntegrationBus
    FILES_MATCHING PATTERN *.hpp
)

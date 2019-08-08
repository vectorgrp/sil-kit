add_library(IbInterface INTERFACE)

# Use generator expression to have a different set of include paths when the target is built versus when the target is installed somewhere
target_include_directories(IbInterface
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
)


if (TARGET fmt-header-only)
    message(STATUS "have fmt-header-only")
    target_link_libraries(IbInterface
        INTERFACE
            $<BUILD_INTERFACE:fmt-header-only>
            $<INSTALL_INTERFACE:fmt-header-only>
    )
    target_compile_definitions(IbInterface
        INTERFACE
            $<BUILD_INTERFACE:HAVE_FMTLIB>
            $<BUILD_INTERFACE:FMT_HEADER_ONLY>
    )
else()
    message(FATAL_ERROR "Foo")
endif()

# Add a custom project IbApi for quick access to IB Headers in Visual Studio
file(GLOB_RECURSE IB_INTERFACE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.hpp)
add_custom_target(IbApi SOURCES ${IB_INTERFACE_SOURCES})
if(CMAKE_VERSION VERSION_GREATER 3.8.0)
    # Easy access to IntegrationBus includes for Visual Studio
    source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/ib PREFIX "ib" FILES ${IB_INTERFACE_SOURCES})
endif()


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
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/
    DESTINATION ${INSTALL_INCLUDE_DIR}
    COMPONENT IntegrationBus
    FILES_MATCHING PATTERN *.hpp
)

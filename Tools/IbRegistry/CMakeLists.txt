set(CMAKE_CXX_STANDARD 14)

add_executable(IbRegistry
    IbRegistry.cpp
)

# Group this demo project into a folder
set_target_properties(IbRegistry PROPERTIES
     FOLDER "Tools"
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Tools/$<CONFIG>/bin"
)

# we're linking statically with IB components and not with the dll.
add_definitions(-DEXPORT_IntegrationBusAPI)

target_link_libraries(IbRegistry
    IbMwVAsio
    IbConfig
)

install(TARGETS IbRegistry
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

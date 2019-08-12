set(CMAKE_CXX_STANDARD 14)

project("IntegrationBus-Registry" LANGUAGES CXX C)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/IntegrationBusInstall.cmake)
configure_ib_install(${PROJECT_NAME})

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
    RUNTIME DESTINATION ${INSTALL_BINDIR}
)

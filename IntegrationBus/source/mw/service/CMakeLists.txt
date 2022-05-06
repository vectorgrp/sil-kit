# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbMwService STATIC
    ServiceDatatypes.hpp
    ServiceDiscovery.hpp
    ServiceDiscovery.cpp
    SpecificDiscoveryStore.cpp
    SpecificDiscoveryStore.hpp
)

target_include_directories(IbMwService
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(IbMwService
    PUBLIC IbInterface
    PRIVATE 
        IbInternal
        IbConfiguration
        IbMwLogging
    INTERFACE 
        IbInternal
        IbConfiguration
)


add_vib_test(TestMwServiceDiscovery
    SOURCES ServiceDiscoveryTest.cpp 
    LIBS IbMwService IbMwMockParticipant IbUuid)

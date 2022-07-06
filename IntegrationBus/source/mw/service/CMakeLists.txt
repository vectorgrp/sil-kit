# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitMwService STATIC
    ServiceDatatypes.hpp
    ServiceDiscovery.hpp
    ServiceDiscovery.cpp
    SpecificDiscoveryStore.cpp
    SpecificDiscoveryStore.hpp

    ServiceSerdes.hpp
    ServiceSerdes.cpp
)

target_include_directories(SilKitMwService
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(SilKitMwService
    PUBLIC SilKitInterface
    PRIVATE 
        SilKitInternal
        SilKitConfiguration
        SilKitMwLogging
    INTERFACE 
        SilKitInternal
        SilKitConfiguration
)


add_silkit_test(TestMwServiceDiscovery
    SOURCES ServiceDiscoveryTest.cpp 
    LIBS SilKitMwService SilKitMwMockParticipant SilKitUuid)

add_silkit_test(TestMwService_Serdes SOURCES ServiceSerdesTest.cpp LIBS SilKitMwService)

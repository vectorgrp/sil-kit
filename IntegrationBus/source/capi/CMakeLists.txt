# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_vib_test(TestCapiCan SOURCES CapiCanTest.cpp LIBS IbSimCan IntegrationBus)
set_target_properties(TestCapiCan PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_vib_test(TestCapiEthernet SOURCES CapiEthernetTest.cpp LIBS IbSimEthernet IntegrationBus)
set_target_properties(TestCapiEthernet PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_vib_test(TestCapiFlexRay SOURCES CapiFlexRayTest.cpp LIBS IbSimFlexRay IbMwMockComAdapter IntegrationBus)
set_target_properties(TestCapiFlexRay PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_vib_test(TestCapiData SOURCES CapiDataTest.cpp LIBS IbSimGenericMessage IbMwMockComAdapter IntegrationBus)
set_target_properties(TestCapiData PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_vib_test(TestCapiIntegrationBus SOURCES CapiIntegrationBusTest.cpp LIBS IbMwMockComAdapter IntegrationBus)
set_target_properties(TestCapiIntegrationBus PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_vib_test(TestCapiParticipantStateHandling SOURCES CapiParticipantStateHandlingTest.cpp LIBS IbMwMockComAdapter IntegrationBus)
set_target_properties(TestCapiParticipantStateHandling PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_vib_test(TestCapiTimeSync SOURCES CapiTimeSyncTest.cpp LIBS IbMwMockComAdapter IntegrationBus)
set_target_properties(TestCapiTimeSync PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)
# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_vib_test(TestCapiCan SOURCES CapiCanTest.cpp LIBS IbSimCan IbMwMockComAdapter IntegrationBus)
add_vib_test(TestCapiEthernet SOURCES CapiEthernetTest.cpp LIBS IbSimEthernet IntegrationBus)
add_vib_test(TestCapiFlexRay SOURCES CapiFlexRayTest.cpp LIBS IbSimFlexRay IbMwMockComAdapter IntegrationBus)
add_vib_test(TestCapiData SOURCES CapiDataTest.cpp LIBS IbSimGenericMessage IbMwMockComAdapter IntegrationBus)
set_target_properties(TestCapiData PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)
add_vib_test(TestCapiRpc SOURCES CapiRpcTest.cpp LIBS IbSimRpc IbMwMockComAdapter IntegrationBus)

add_vib_test(TestCapiLogger SOURCES CapiLoggerTest.cpp LIBS IbMwLogging IntegrationBus)
set_target_properties(TestCapiLogger PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_vib_test(TestCapiIntegrationBus SOURCES CapiIntegrationBusTest.cpp LIBS IbMwMockComAdapter IntegrationBus)
add_vib_test(TestCapiParticipantStateHandling SOURCES CapiParticipantStateHandlingTest.cpp LIBS IbMwMockComAdapter IntegrationBus)
add_vib_test(TestCapiTimeSync SOURCES CapiTimeSyncTest.cpp LIBS IbMwMockComAdapter IntegrationBus)
add_vib_test(TestCapiLin SOURCES CapiLinTest.cpp LIBS IbSimLin IbMwMockComAdapter IntegrationBus)


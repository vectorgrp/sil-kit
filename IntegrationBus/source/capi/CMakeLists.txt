# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_vib_test(TestCapiCan SOURCES CapiCanTest.cpp LIBS IbSimCan IntegrationBus)
add_vib_test(TestCapiEthernet SOURCES CapiEthernetTest.cpp LIBS IbSimEthernet IntegrationBus)
add_vib_test(TestCapiFlexRay SOURCES CapiFlexRayTest.cpp LIBS IbSimFlexRay IbMwMockComAdapter IntegrationBus)
add_vib_test(TestCapiData SOURCES CapiDataTest.cpp LIBS IbSimGenericMessage IbMwMockComAdapter IntegrationBus)
add_vib_test(TestCapiIntegrationBus SOURCES CapiIntegrationBusTest.cpp LIBS IbMwMockComAdapter IntegrationBus)
add_vib_test(TestCapiParticipantStateHandling SOURCES CapiParticipantStateHandlingTest.cpp LIBS IbMwMockComAdapter IntegrationBus)
add_vib_test(TestCapiTimeSync SOURCES CapiTimeSyncTest.cpp LIBS IbMwMockComAdapter IntegrationBus)
add_vib_test(TestCapiLin SOURCES CapiLinTest.cpp LIBS IbSimLin IbMwMockComAdapter IntegrationBus)


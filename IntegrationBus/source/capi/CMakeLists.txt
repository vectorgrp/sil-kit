# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_vib_test(TestCapiCan SOURCES CapiCanTest.cpp LIBS IbSimCan IbMwMockParticipant IntegrationBus)
add_vib_test(TestCapiEthernet SOURCES CapiEthernetTest.cpp LIBS IbSimEthernet IbMwMockParticipant IntegrationBus)
add_vib_test(TestCapiFlexray SOURCES CapiFlexrayTest.cpp LIBS IbSimFlexray IbMwMockParticipant IntegrationBus)
add_vib_test(TestCapiData SOURCES CapiDataTest.cpp LIBS IbMwMockParticipant IntegrationBus)
add_vib_test(TestCapiRpc SOURCES CapiRpcTest.cpp LIBS IbSimRpc IbMwMockParticipant IntegrationBus)

add_vib_test(TestCapiLogger SOURCES CapiLoggerTest.cpp LIBS IbMwLogging IntegrationBus)

add_vib_test(TestCapiIntegrationBus SOURCES CapiIntegrationBusTest.cpp LIBS IbMwMockParticipant IntegrationBus)
add_vib_test(TestCapiParticipantStateHandling SOURCES CapiParticipantStateHandlingTest.cpp LIBS IbMwMockParticipant IntegrationBus)
add_vib_test(TestCapiTimeSync SOURCES CapiTimeSyncTest.cpp LIBS IbMwMockParticipant IntegrationBus)
add_vib_test(TestCapiLin SOURCES CapiLinTest.cpp LIBS IbSimLin IbMwMockParticipant IntegrationBus)


# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_silkit_test(TestCapiCan SOURCES CapiCanTest.cpp LIBS SilKitSimCan SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiEthernet SOURCES CapiEthernetTest.cpp LIBS SilKitSimEthernet SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiFlexray SOURCES CapiFlexrayTest.cpp LIBS SilKitSimFlexray SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiData SOURCES CapiDataTest.cpp LIBS SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiRpc SOURCES CapiRpcTest.cpp LIBS SilKitSimRpc SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiLogger SOURCES CapiLoggerTest.cpp LIBS SilKitMwLogging SilKit)

add_silkit_test(TestCapiSilKit SOURCES CapiSilKitTest.cpp LIBS SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiParticipantStateHandling SOURCES CapiParticipantStateHandlingTest.cpp LIBS SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiTimeSync SOURCES CapiTimeSyncTest.cpp LIBS SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiLin SOURCES CapiLinTest.cpp LIBS SilKitSimLin SilKitMwMockParticipant SilKit)


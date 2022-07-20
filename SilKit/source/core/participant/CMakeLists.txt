# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitMwParticipant STATIC
    Participant.hpp
    Participant_impl.hpp
    Participant.cpp
    CreateParticipant_impl.hpp
    CreateParticipant.hpp
    CreateParticipant.cpp
)

target_link_libraries(SilKitMwParticipant
    INTERFACE  SilKitInterface
    PUBLIC
        SilKitInternal
        SilKitMwLogging
        SilKitConfiguration
        SilKitUtil
        SilKitSetThreadName
        SilKitUuid
        SilKitMwSync
        SilKitSimCan
        SilKitSimEthernet
        SilKitSimFlexray
        SilKitSimLin
        SilKitSimDataMessage
        SilKitSimRpc
        SilKitTracing
        SilKitMwService
    INTERFACE  SilKitMwService
    PRIVATE
        SilKitVersion
)

target_link_libraries(SilKitMwParticipant PUBLIC SilKitMwVAsio)

target_include_directories(SilKitMwParticipant
    PUBLIC .
)

if (MSVC)
    target_compile_options(SilKitMwParticipant PRIVATE "/bigobj")
endif (MSVC)

add_silkit_test(TestMwParticipant 
    SOURCES ParticipantTest.cpp 
    LIBS SilKitMwMockNullConnection SilKitConfigurationTestUtils
)

# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbMwParticipant STATIC
    Participant.hpp
    Participant_impl.hpp
    Participant.cpp
    CreateParticipant.hpp
    CreateParticipant.cpp
)

target_link_libraries(IbMwParticipant
    INTERFACE  IbInterface
    PUBLIC
        IbInternal
        IbMwLogging
        IbConfiguration
        IbUtil
        IbSetThreadName
        IbUuid
        IbMwSync
        IbSimCan
        IbSimEthernet
        IbSimFlexray
        IbSimLin
        IbSimDataMessage
        IbSimRpc
        IbTracing
        IbMwService
    INTERFACE  IbMwService
    PRIVATE
        IbVersion
)

target_link_libraries(IbMwParticipant PUBLIC IbMwVAsio)

target_include_directories(IbMwParticipant
    PUBLIC .
)

if (MSVC)
    target_compile_options(IbMwParticipant PRIVATE "/bigobj")
endif (MSVC)

add_vib_test(TestMwParticipant 
    SOURCES ParticipantTest.cpp 
    LIBS IbMwMockNullConnection IbConfigurationTestUtils
)

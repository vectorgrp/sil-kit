# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimDataMessage STATIC
    DataMessageDatatypeUtils.hpp
    DataMessageDatatypeUtils.cpp
    DataPublisher.hpp
    DataPublisher.cpp
    DataSubscriber.hpp
    DataSubscriber.cpp

    DataSubscriberInternal.hpp
    DataSubscriberInternal.cpp

    DataSerdes.hpp
    DataSerdes.cpp
)

target_include_directories(IbSimDataMessage
    PUBLIC .
)

target_link_libraries(IbSimDataMessage
    PRIVATE IbInterface
            IbSetThreadName
            IbInternal
            IbConfiguration
            IbUtil
)

add_vib_test(TestSimDataPublisher 
    SOURCES DataPublisherTest.cpp 
    LIBS IbSimDataMessage IbMwMockParticipant IbInternal 
)
add_vib_test(TestSimDataSubscriber
    SOURCES DataSubscriberTest.cpp
    LIBS IbSimDataMessage IbMwMockParticipant IbInternal 
)
add_vib_test(TestSimDataSubscriberInternal
    SOURCES DataSubscriberInternalTest.cpp
    LIBS IbSimDataMessage IbMwMockParticipant IbInternal
)
add_vib_test(TestPubSubMatching
    SOURCES PubSubMatchingTest.cpp 
    LIBS IbSimDataMessage IbUtil
)
add_vib_test(TestSimData_Serdes SOURCES DataSerdesTest.cpp LIBS IbSimDataMessage IbInternal)

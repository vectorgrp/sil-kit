# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitSimDataMessage STATIC
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

target_include_directories(SilKitSimDataMessage
    PUBLIC .
)

target_link_libraries(SilKitSimDataMessage
    PRIVATE SilKitInterface
            SilKitSetThreadName
            SilKitInternal
            SilKitConfiguration
            SilKitUtil
)

add_silkit_test(TestSimDataPublisher 
    SOURCES DataPublisherTest.cpp 
    LIBS SilKitSimDataMessage SilKitMwMockParticipant SilKitInternal 
)
add_silkit_test(TestSimDataSubscriber
    SOURCES DataSubscriberTest.cpp
    LIBS SilKitSimDataMessage SilKitMwMockParticipant SilKitInternal 
)
add_silkit_test(TestSimDataSubscriberInternal
    SOURCES DataSubscriberInternalTest.cpp
    LIBS SilKitSimDataMessage SilKitMwMockParticipant SilKitInternal
)
add_silkit_test(TestPubSubMatching
    SOURCES PubSubMatchingTest.cpp 
    LIBS SilKitSimDataMessage SilKitUtil
)
add_silkit_test(TestSimData_Serdes SOURCES DataSerdesTest.cpp LIBS SilKitSimDataMessage SilKitInternal)

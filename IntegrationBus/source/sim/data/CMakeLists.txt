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
)

target_include_directories(IbSimDataMessage
    PUBLIC .
)

target_link_libraries(IbSimDataMessage
    PRIVATE IbInterface
            IbSetThreadName
            IbInternal
            IbConfiguration
)

add_vib_test(TestSimDataPublisher 
    SOURCES DataPublisherTest.cpp 
    LIBS IbSimDataMessage IbMwMockComAdapter IbInternal
)
add_vib_test(TestSimDataSubscriber 
    SOURCES DataSubscriberTest.cpp 
    LIBS IbSimDataMessage IbMwMockComAdapter IbInternal
)
add_vib_test(TestDataExchangeFormat 
    SOURCES DataExchangeFormatTest.cpp 
    LIBS IbSimDataMessage
)

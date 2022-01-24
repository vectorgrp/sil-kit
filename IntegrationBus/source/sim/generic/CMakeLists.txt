# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimGenericMessage STATIC
    GenericMessageDatatypeUtils.hpp
    GenericMessageDatatypeUtils.cpp
    GenericPublisher.hpp
    GenericPublisher.cpp
    GenericSubscriber.hpp
    GenericSubscriber.cpp

    GenericPublisherReplay.hpp
    GenericPublisherReplay.cpp
    GenericSubscriberReplay.hpp
    GenericSubscriberReplay.cpp
)

target_include_directories(IbSimGenericMessage
    PUBLIC .
)

target_link_libraries(IbSimGenericMessage
    PRIVATE IbInterface
            IbTracing
    INTERFACE IbInternal
)

add_vib_test(TestSimGenericPublisher SOURCES GenericPublisherTest.cpp LIBS IbSimGenericMessage IbConfig_deprecated IbMwMockComAdapter IbTracing)
add_vib_test(TestSimGenericSubscriber SOURCES GenericSubscriberTest.cpp LIBS IbSimGenericMessage IbConfig_deprecated IbMwMockComAdapter IbTracing)

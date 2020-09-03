add_library(IbSimGenericMessage STATIC
    GenericMessageDatatypeUtils.hpp
    GenericMessageDatatypeUtils.cpp
    GenericPublisher.hpp
    GenericPublisher.cpp
    GenericSubscriber.hpp
    GenericSubscriber.cpp
)

target_include_directories(IbSimGenericMessage
    PUBLIC .
)

target_link_libraries(IbSimGenericMessage
    PRIVATE IbInterface
            IbTracing
)

add_vib_test(TestSimGenericPublisher SOURCES GenericPublisherTest.cpp LIBS IbSimGenericMessage IbConfig IbMwMockComAdapter IbTracing)
add_vib_test(TestSimGenericSubscriber SOURCES GenericSubscriberTest.cpp LIBS IbSimGenericMessage IbConfig IbMwMockComAdapter IbTracing)

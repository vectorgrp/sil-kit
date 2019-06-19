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
)

add_mvsc_ib_include_filter()

add_vib_test(TestGenericPublisher SOURCES GenericPublisherTest.cpp LIBS IbSimGenericMessage IbConfig IbMwMock)
add_vib_test(TestGenericSubscriber SOURCES GenericSubscriberTest.cpp LIBS IbSimGenericMessage IbConfig IbMwMock)

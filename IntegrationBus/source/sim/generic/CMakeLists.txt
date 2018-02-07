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

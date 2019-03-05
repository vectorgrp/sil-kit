project("IbMwFastRtpsIdl" CXX C)

add_library(${PROJECT_NAME} STATIC
    all.hpp
    CanTopics.cxx
    CanTopics.h
    CanTopicsPubSubTypes.cxx
    CanTopicsPubSubTypes.h
    Common.cxx
    Common.h
    CommonPubSubTypes.cxx
    CommonPubSubTypes.h
    EthernetTopics.cxx
    EthernetTopics.h
    EthernetTopicsPubSubTypes.cxx
    EthernetTopicsPubSubTypes.h
    FlexRayTopics.cxx
    FlexRayTopics.h
    FlexRayTopicsPubSubTypes.cxx
    FlexRayTopicsPubSubTypes.h
    GenericTopics.cxx
    GenericTopics.h
    GenericTopicsPubSubTypes.h
    GenericTopicsPubSubTypes.cxx
    IoTopics.cxx
    IoTopics.h
    IoTopicsPubSubTypes.cxx
    IoTopicsPubSubTypes.h
    LinTopics.cxx
    LinTopics.h
    LinTopicsPubSubTypes.cxx
    LinTopicsPubSubTypes.h
    LoggingTopics.cxx
    LoggingTopics.h
    LoggingTopicsPubSubTypes.cxx
    LoggingTopicsPubSubTypes.h
    MiddlewareTopics.cxx
    MiddlewareTopics.h
    MiddlewareTopicsPubSubTypes.cxx
    MiddlewareTopicsPubSubTypes.h
    )

source_group("Abstract Interfaces" REGULAR_EXPRESSION "Interfaces")

target_link_libraries(${PROJECT_NAME}
    fastrtps
    fastcdr
  )

target_include_directories(${PROJECT_NAME}
  PRIVATE ${IB_PROJECT_DIR}
)

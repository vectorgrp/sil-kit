# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbMwService STATIC
    ServiceDatatypes.hpp
    ServiceDiscovery.hpp
    ServiceDiscovery.cpp
)

target_include_directories(IbMwService
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(IbMwService
    PUBLIC IbInterface
    PRIVATE IbInternal
)


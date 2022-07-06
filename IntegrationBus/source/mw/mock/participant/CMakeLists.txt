# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitMwMockParticipant INTERFACE
)

target_include_directories(SilKitMwMockParticipant
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(SilKitMwMockParticipant
    INTERFACE SilKitMwLogging SilKitMwSync SilKitInternal
)

# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbMwMockParticipant INTERFACE
)

target_include_directories(IbMwMockParticipant
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(IbMwMockParticipant
    INTERFACE IbMwLogging IbMwSync IbInternal
)

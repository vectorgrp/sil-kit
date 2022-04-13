# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbMwMockNullConnection STATIC
    NullConnectionParticipant.cpp
    NullConnectionParticipant.hpp
)

target_include_directories(IbMwMockNullConnection
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(IbMwMockNullConnection
    PUBLIC IbMwParticipant
)

if (MSVC)
    target_compile_options(IbMwMockNullConnection PRIVATE "/bigobj")
endif (MSVC)

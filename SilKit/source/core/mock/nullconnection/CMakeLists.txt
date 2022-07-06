# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitMwMockNullConnection STATIC
    NullConnectionParticipant.cpp
    NullConnectionParticipant.hpp
)

target_include_directories(SilKitMwMockNullConnection
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(SilKitMwMockNullConnection
    PUBLIC SilKitMwParticipant
)

if (MSVC)
    target_compile_options(SilKitMwMockNullConnection PRIVATE "/bigobj")
endif (MSVC)

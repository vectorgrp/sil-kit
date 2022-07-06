# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitUtil INTERFACE)

target_include_directories(SilKitUtil
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_subdirectory(tests)

# Filesystem utilities required for cross platform and language portability.
# Can be removed when we finaly upgrade to C++17

add_library(SilKitFilesystem STATIC
    Filesystem.hpp
    Filesystem.cpp
)
target_include_directories(SilKitFilesystem
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_library(SilKitSetThreadName STATIC
    SetThreadName.hpp
    SetThreadName.cpp
)
target_include_directories(SilKitSetThreadName
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_library(SilKitUuid STATIC
    UuidRandom.hpp
    UuidRandom.cpp
)
target_include_directories(SilKitUuid
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

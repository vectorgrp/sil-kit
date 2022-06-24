# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbUtil INTERFACE)

target_include_directories(IbUtil
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_subdirectory(tests)

# Filesystem utilities required for cross platform and language portability.
# Can be removed when we finaly upgrade to C++17

add_library(IbFilesystem STATIC
    Filesystem.hpp
    Filesystem.cpp
)
target_include_directories(IbFilesystem
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_library(IbSetThreadName STATIC
    SetThreadName.hpp
    SetThreadName.cpp
)
target_include_directories(IbSetThreadName
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_library(IbUuid STATIC
    UuidRandom.hpp
    UuidRandom.cpp
)
target_include_directories(IbUuid
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

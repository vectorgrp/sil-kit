# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_subdirectory(internal)
add_subdirectory(service)
add_subdirectory(participant)
add_subdirectory(vasio)
if(SILKIT_BUILD_TESTS)
    add_subdirectory(mock)
endif()
add_subdirectory(sync)

# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitSimTestHarness
    SimSystemController.hpp
    SimTestHarness.hpp
    SimTestHarness.cpp
)
target_link_libraries(SilKitSimTestHarness
    PRIVATE
        SilKit
    PUBLIC
        SilKitConfigurationTestUtils
)
target_include_directories(SilKitSimTestHarness
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)
set_property(TARGET SilKitSimTestHarness PROPERTY FOLDER "Tests")

set_target_properties(SilKitSimTestHarness PROPERTIES
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)


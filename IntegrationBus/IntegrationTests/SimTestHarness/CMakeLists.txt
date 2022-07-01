# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimTestHarness
    SimSystemController.hpp
    SimTestHarness.hpp
    SimTestHarness.cpp
)
target_link_libraries(IbSimTestHarness
    PRIVATE
        IntegrationBus
    PUBLIC
        IbConfigurationTestUtils
)
target_include_directories(IbSimTestHarness
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)
set_property(TARGET IbSimTestHarness PROPERTY FOLDER "Tests")

set_target_properties(IbSimTestHarness PROPERTIES
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)


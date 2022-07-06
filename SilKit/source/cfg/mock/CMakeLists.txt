# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitConfigurationTestUtils INTERFACE
    ConfigurationTestUtils.hpp
)

target_include_directories(SilKitConfigurationTestUtils
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(SilKitConfigurationTestUtils
    INTERFACE SilKitConfiguration
)

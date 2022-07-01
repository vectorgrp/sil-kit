# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbConfigurationTestUtils INTERFACE
    ConfigurationTestUtils.hpp
)

target_include_directories(IbConfigurationTestUtils
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(IbConfigurationTestUtils
    INTERFACE IbConfiguration
)

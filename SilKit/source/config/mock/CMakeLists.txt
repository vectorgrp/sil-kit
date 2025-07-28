# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Config_TestUtils INTERFACE
    ConfigurationTestUtils.hpp
)

target_include_directories(I_SilKit_Config_TestUtils
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(I_SilKit_Config_TestUtils
    INTERFACE I_SilKit_Config
)

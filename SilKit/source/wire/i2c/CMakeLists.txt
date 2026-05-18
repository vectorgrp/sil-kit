# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Wire_I2c INTERFACE)
target_include_directories(I_SilKit_Wire_I2c INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(I_SilKit_Wire_I2c INTERFACE I_SilKit_Wire_Util)

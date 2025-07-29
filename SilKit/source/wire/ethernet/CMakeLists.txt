# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Wire_Ethernet INTERFACE)
target_include_directories(I_SilKit_Wire_Ethernet INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(I_SilKit_Wire_Ethernet INTERFACE I_SilKit_Wire_Util)

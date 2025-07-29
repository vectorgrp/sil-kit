# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

#NB: Call <<QNX_INSTALLATION_DIR>>/qnxsdp-env.sh before building
set(SILKIT_TARGET_TOOLSET "gcc_ntox86_64")
set(SILKIT_TARGET_ARCHITECTURE "x86_64")
include(${CMAKE_CURRENT_LIST_DIR}/qnx-cross-base-toolchain.cmake)

# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

#NB: Call <<QNX_INSTALLATION_DIR>>/qnxsdp-env.sh before building
set(SILKIT_TARGET_TOOLSET "gcc_ntoarmv7le_gpp")
set(SILKIT_TARGET_ARCHITECTURE "armv7")
include(${CMAKE_CURRENT_LIST_DIR}/qnx-cross-base-toolchain.cmake)

# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# On Debian install : gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_CROSSCOMPILING TRUE)
set(TOOL_TRIPLE arm-linux-gnueabihf)
set(CMAKE_C_COMPILER   ${TOOL_TRIPLE}-gcc)
set(CMAKE_CXX_COMPILER ${TOOL_TRIPLE}-g++)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


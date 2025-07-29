# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# Crossbuild from Ubuntu to MinGW using the 'mingw-w64' package
set(CMAKE_SYSTEM_NAME Windows)
set(TOOL_TRIPLE x86_64-w64-mingw32)
#set(THREAD_FLAVOR_SUFFIX "")
set(THREAD_FLAVOR_SUFFIX "-posix") #use winpthreads
#set(THREAD_FLAVOR_SUFFIX "-win32") #Use win32 threads
set(CMAKE_C_COMPILER   ${TOOL_TRIPLE}-gcc${THREAD_FLAVOR_SUFFIX})
set(CMAKE_CXX_COMPILER ${TOOL_TRIPLE}-g++${THREAD_FLAVOR_SUFFIX})
set(CMAKE_RC_COMPILER ${TOOL_TRIPLE}-windres)

# Handling the pthread depencency on Win32:
# - fix missing symbols when using winpthreads:
set(_c_flags "-static-libgcc -static-libstdc++ -m64")
# - statically link winpthread, so we do not have a shared library
#   dependency on SilKit.dll
set(_l_flags "-Wl,-Bstatic,--whole-archive -lwinpthread -Wl,-Bdynamic,--no-whole-archive")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${_c_flags} ${_l_flags}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_c_flags} ${_l_flags}")
#ensure assembler does not run out of space when building Debug binaries
# - use optimizations to reduce binary size:
set(CMAKE_CXX_FLAGS_DEBUG_INIT "${_c_flags} -g3 -O2 ")
# - tell the assembler to use big objects:
set(_asm_flags "-m64 -Wa,-mbig-obj" )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_asm_flags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_asm_flags}")


# where is the target environment located
set(CMAKE_FIND_ROOT_PATH  /usr/${TOOL_TRIPLE})

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# yaml-cpp does some prefix calculations based on this:
set(CMAKE_INSTALL_PREFIX "" CACHE PATH "Ensure yaml-cpp is crossbuildable" FORCE)

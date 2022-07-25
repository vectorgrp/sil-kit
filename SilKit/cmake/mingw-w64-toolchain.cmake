# Copyright (c) 2022 Vector Informatik GmbH
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

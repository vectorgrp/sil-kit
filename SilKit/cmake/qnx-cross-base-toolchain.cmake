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

#NB: Call <<QNX_INSTALLATION_DIR>>/qnxsdp-env.sh before building
set(CMAKE_SYSTEM_NAME QNX)
set(CMAKE_CROSSCOMPILING TRUE)

if(NOT DEFINED ENV{QNX_HOST})
    message(FATAL_ERROR "SIL Kit: QNX cross-build: please invoke/source \
        <<QNX Software Development Platform Path>>/qnxsdp-env.{sh,bat} before running CMake.")
endif()

if(NOT DEFINED SILKIT_TARGET_TOOLSET)
    message(NOTICE "SIL Kit: QNX cross-build: please specify SILKIT_TARGET_TOOLSET \
                as a CMake variable. see `qcc -V` for a list of available targets.\
                For Example: gcc_ntox86_64_gpp.")
endif()
set(SILKIT_TARGET_TOOLSET "${SILKIT_TARGET_TOOLSET}" CACHE STRING "SIL Kit target architecture" FORCE)


# NB: see `qcc -V` for a list of targets, `_gpp` is GNU C++ library
set(qcc_arch ${SILKIT_TARGET_TOOLSET})
set(target_arch ${SILKIT_TARGET_ARCHITECTURE})
set(CMAKE_SYSTEM_PROCESSOR ${target_arch})

# find QCC and friends
find_program(QCC_EXE qcc HINTS $ENV{QNX_HOST}/usr/bin REQUIRED)
find_program(QPP_EXE q++ HINTS $ENV{QNX_HOST}/usr/bin REQUIRED)
find_program(RANLIB_EXE nto${target_arch}-ranlib HINTS $ENV{QNX_HOST}/usr/bin REQUIRED)
find_program(AR_EXE nto${target_arch}-ar HINTS $ENV{QNX_HOST}/usr/bin REQUIRED)
find_program(MAKE_EXE make HINTS $ENV{QNX_HOST}/usr/bin  /bin /usr/bin REQUIRED)

message(STATUS "SIL Kit: QNX cross-build: QNX_HOST=$ENV{QNX_HOST}")
message(STATUS "SIL Kit: QNX cross-build: qcc executable: ${QCC_EXE}")
message(STATUS "SIL Kit: QNX cross-build: q++ executable: ${QPP_EXE}")
message(STATUS "SIL Kit: QNX cross-build: ranlib executable: ${RANLIB_EXE}")
message(STATUS "SIL Kit: QNX cross-build: ar executable: ${AR_EXE}")
message(STATUS "SIL Kit: QNX cross-build: make executable: ${MAKE_EXE}")
message(STATUS "SIL Kit: QNX cross-build: SILKIT_TARGET_TOOLSET=${target_arch}")
message(STATUS "SIL Kit: QNX cross-build: qcc flags ${qcc_arch}")


#valid target_arch: x86_64 aarch64 armv7
set(CMAKE_SIZEOF_VOID_P 8)
if(${target_arch} MATCHES armv7)
    set(CMAKE_SIZEOF_VOID_P 4)
endif()

# Add intrinsic QNX dependencies/flags here
add_compile_definitions(_QNX_SOURCE) #include all non-posix headers
link_libraries(-lsocket) # link against QNX TCP/IP Stack

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_C_COMPILER ${QCC_EXE})
set(CMAKE_C_COMPILER_TARGET ${qcc_arch})

set(CMAKE_CXX_COMPILER ${QPP_EXE})
set(CMAKE_CXX_COMPILER_TARGET ${qcc_arch})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wc,-std=c++14")

set(CMAKE_ASM_COMPILER "${QCC_EXE}" -V${qcc_arch})
set(CMAKE_ASM_DEFINE_FLAG "-Wa,--defsym,")

set(CMAKE_RANLIB ${RANLIB_EXE} CACHE PATH "QNX ranlib" FORCE)
set(CMAKE_AR ${AR_EXE} CACHE PATH "QNX ranlib" FORCE)

set(CMAKE_MAKE_PROGRAM ${MAKE_EXE} CACHE PATH "QNX make" FORCE)

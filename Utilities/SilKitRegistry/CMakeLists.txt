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

cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 14)

include(SilKitInstall)

if(MSVC)
    #disable warnings on visual studio 2015 v14
    if(MSVC_VERSION EQUAL 1900)
        add_compile_options("/wd4503")
    endif()
endif (MSVC)

add_subdirectory(config)

add_executable(sil-kit-registry
    Registry.cpp

    WindowsServiceMain.hpp
    WindowsServiceMain.cpp

    $<$<CXX_COMPILER_ID:MSVC>: "${CMAKE_CURRENT_SOURCE_DIR}/../utilities.manifest" >
)

# Group this demo project into a folder
set_target_properties(sil-kit-registry PROPERTIES
     FOLDER "Utilities"
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

# We're linking statically with SIL Kit components and not with the dll.
add_definitions(-DEXPORT_SilKitAPI)

target_link_libraries(sil-kit-registry
    PRIVATE
    I_SilKit_Core_Participant #ValidateAndSanitize
    I_SilKit_Util

    S_SilKitImpl
    O_SilKit_Dashboard

    O_SilKitRegistry_Config

    O_SilKit_Util_SignalHandler
)

# Set versioning infos on exe
if(MSVC)
    get_target_property(SILKIT_BINARY_DIR SilKit BINARY_DIR)
    get_target_property(SILKIT_SOURCE_DIR SilKit SOURCE_DIR)
    # Include the generated version_macros.hpp in SilKit/source
    target_include_directories(sil-kit-registry
        PRIVATE ${SILKIT_BINARY_DIR}
        PRIVATE ${SILKIT_SOURCE_DIR}
    )
    target_sources(sil-kit-registry PRIVATE SilKitRegistry.rc)
endif()

install(TARGETS sil-kit-registry
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    COMPONENT utils
)

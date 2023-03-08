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

# the SilKitExtension is a standalone library except for its dependency on
# silkit/SilKitVersion.hpp

add_library(S_SilKit_Extensions STATIC
    SilKitExtensions.cpp
    SilKitExtensions.hpp
    detail/LoadExtension.hpp
    SilKitExtensionApi/ISilKitExtension.hpp
    SilKitExtensionApi/SilKitExtensionBase.hpp
    SilKitExtensionApi/SilKitExtensionMacros.hpp
    SilKitExtensionApi/SilKitExtensionABI.h
    SilKitExtensionApi/SilKitExtensionUtils.hpp


    # Imlplementation Helpers
    SilKitExtensionImpl/DllCache.hpp
    SilKitExtensionImpl/SilKitExtensionLoader.hpp
    #Extension implementations
    #   MDF4 (private)
    SilKitExtensionImpl/CreateMdf4Tracing.cpp
    SilKitExtensionImpl/CreateMdf4Tracing.hpp
)

target_include_directories(S_SilKit_Extensions
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/SilKitExtensionApi
    ${CMAKE_CURRENT_LIST_DIR}/SilKitExtensionImpl
)

target_link_libraries(S_SilKit_Extensions
    PUBLIC  SilKitInterface
            I_SilKit_Config
    PRIVATE SilKitVersion
            S_SilKit_Tracing
)

#Reproducible build: do not include CMAKE_SYSTEM_VERSION, it contains kernel version
# and build ID.
set(extension_os "${CMAKE_SYSTEM_NAME}/${SILKIT_HOST_PLATFORM}")

if(UNIX)
    target_link_libraries(S_SilKit_Extensions
        PRIVATE ${CMAKE_DL_LIBS}
    )

    target_sources(S_SilKit_Extensions PRIVATE detail/LoadExtension_posix.cpp)

    #make sure we don't compile obviously bad code
    target_compile_options(S_SilKit_Extensions
        PRIVATE
            -pedantic -Wall -Wextra
            -Wcast-align -Wformat=2 -Wmissing-declarations
            -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5
            -Wswitch-default -Wundef -Wno-unused
            -Wpacked
            -Werror -Wfatal-errors
    )

elseif(MSVC OR MINGW)

    target_sources(S_SilKit_Extensions PRIVATE detail/LoadExtension_win.cpp)

    if(DEFINED MSVC_TOOLSET_VERSION)
        target_compile_definitions(S_SilKit_Extensions
            PUBLIC
                SILKIT_MSVC_TOOLSET_VERSION=${MSVC_TOOLSET_VERSION}
            )

    endif()
endif()

target_compile_definitions(S_SilKit_Extensions
    PRIVATE
    SILKIT_EXTENSION_OS="${extension_os}"
)

if(SILKIT_BUILD_TESTS)
    add_subdirectory(Tests)
endif()


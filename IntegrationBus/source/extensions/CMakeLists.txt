# Copyright (c) Vector Informatik GmbH. All rights reserved.
# the SilKitExtension is a standalone library except for its dependency on
# silkit/version.hpp

add_library(SilKitExtensions STATIC
    SilKitExtensions.cpp
    SilKitExtensions.hpp
    detail/LoadExtension.hpp
    SilKitExtensionApi/ISilKitExtension.hpp
    SilKitExtensionApi/SilKitExtensionBase.hpp
    SilKitExtensionApi/SilKitExtensionMacros.hpp
    SilKitExtensionApi/SilKitExtensionABI.h
    SilKitExtensionApi/SilKitExtensionUtils.hpp
    SilKitExtensionApi/SilKitExtensionUtils.cpp


    # Imlplementation Helpers
    SilKitExtensionImpl/DllCache.hpp
    SilKitExtensionImpl/FactorySingleton.hpp
    #Extension implementations
    #   MDF4 (private)
    SilKitExtensionImpl/CreateMdf4Tracing.cpp
    SilKitExtensionImpl/CreateMdf4Tracing.hpp
)

target_include_directories(SilKitExtensions
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/SilKitExtensionApi
    ${CMAKE_CURRENT_LIST_DIR}/SilKitExtensionImpl
)

target_link_libraries(SilKitExtensions
    PUBLIC  SilKitInterface
            SilKitConfiguration
    PRIVATE SilKitVersion
            SilKitTracing
)

#Reproducible build: do not include CMAKE_SYSTEM_VERSION, it contains kernel version
# and build ID.
set(extension_os "${CMAKE_SYSTEM_NAME}")

if(UNIX)
    include(get_compiler_arch)
    get_linux_distro(ubuntu_vers)
    if(DEFINED ubuntu_vers)
        set(extension_os "${extension_os}/${ubuntu_vers}")
    endif()

    target_link_libraries(SilKitExtensions
        PRIVATE ${CMAKE_DL_LIBS}
    )

    target_sources(SilKitExtensions PRIVATE detail/LoadExtension_posix.cpp)

    #make sure we don't compile obviously bad code
    target_compile_options(SilKitExtensions
        PRIVATE
            -pedantic -Wall -Wextra
            -Wcast-align -Wformat=2 -Wmissing-declarations
            -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5
            -Wswitch-default -Wundef -Wno-unused
            -Wpacked
            -Werror -Wfatal-errors
    )

elseif(MSVC OR MINGW)

    target_sources(SilKitExtensions PRIVATE detail/LoadExtension_win.cpp)

    if(DEFINED MSVC_TOOLSET_VERSION)
        target_compile_definitions(SilKitExtensions
            PUBLIC
                SILKIT_MSVC_TOOLSET_VERSION=${MSVC_TOOLSET_VERSION}
            )

    endif()
endif()

target_compile_definitions(SilKitExtensions
    PRIVATE
    SILKIT_EXTENSION_OS="${extension_os}"
)

if(SILKIT_BUILD_TESTS)
    add_subdirectory(Tests)
endif()


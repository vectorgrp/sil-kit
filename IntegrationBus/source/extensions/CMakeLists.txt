# Copyright (c) Vector Informatik GmbH. All rights reserved.
# the IbExtension is a standalone library except for its dependency on
# ib/version.hpp

add_library(IbExtensions STATIC
    IbExtensions.cpp
    IbExtensions.hpp
    detail/LoadExtension.hpp
    IbExtensionApi/IIbExtension.hpp
    IbExtensionApi/IbExtensionBase.hpp
    IbExtensionApi/IbExtensionMacros.hpp
    IbExtensionApi/IbExtensionABI.h
    IbExtensionApi/IbExtensionUtils.hpp
    IbExtensionApi/IbExtensionUtils.cpp


    # Imlplementation Helpers
    IbExtensionImpl/DllCache.hpp
    IbExtensionImpl/FactorySingleton.hpp
    #Extension implementations
    #   MDF4 (private)
    IbExtensionImpl/CreateMdf4Tracing.cpp
    IbExtensionImpl/CreateMdf4Tracing.hpp
    #   IbRegistry  headers, the implementation is linked inside of
    #   IntegrationBus target
    #      for IDE:
    ${CMAKE_CURRENT_LIST_DIR}/../../include/ib/extensions/IIbRegistry.hpp
    ${CMAKE_CURRENT_LIST_DIR}/../../include/ib/extensions/CreateExtension.hpp
    ${CMAKE_CURRENT_LIST_DIR}/../../include/ib/extensions/ExtensionError.hpp
    IbExtensionImpl/IIbRegistry.hpp
)

target_include_directories(IbExtensions
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/IbExtensionApi
    ${CMAKE_CURRENT_LIST_DIR}/IbExtensionImpl
)

target_link_libraries(IbExtensions
    PUBLIC  IbInterface
            IbConfig_deprecated
            IbConfiguration
    PRIVATE IbVersion
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

    target_link_libraries(IbExtensions
        PRIVATE ${CMAKE_DL_LIBS}
    )

    target_sources(IbExtensions PRIVATE detail/LoadExtension_posix.cpp)

    #make sure we don't compile obviously bad code
    target_compile_options(IbExtensions 
        PRIVATE
            -pedantic -Wall -Wextra
            -Wcast-align -Wformat=2 -Wmissing-declarations
            -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5
            -Wswitch-default -Wundef -Wno-unused
            -Wpacked
            -Werror -Wfatal-errors
    )

elseif(MSVC)

    target_sources(IbExtensions PRIVATE detail/LoadExtension_win.cpp)

    if(DEFINED MSVC_TOOLSET_VERSION)
        target_compile_definitions(IbExtensions
            PUBLIC
                IB_MSVC_TOOLSET_VERSION=${MSVC_TOOLSET_VERSION}
            )

    endif()
endif()

target_compile_definitions(IbExtensions
    PRIVATE
    IB_EXTENSION_OS="${extension_os}"
)

add_subdirectory(Tests)


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

    #private extensions:
    #   MDF4
    IbExtensionImpl/CreateMdf4tracing.cpp
    IbExtensionImpl/CreateMdf4tracing.hpp
)

target_include_directories(IbExtensions
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(IbExtensions
    PUBLIC  IbInterface
            IbConfig
    PRIVATE IbVersion
)

set(extension_os "${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}")

if(UNIX)
    include(get_compiler_arch)
    get_ubuntu_version(ubuntu_vers)
    if(DEFINED ubuntu_vers)
        set(extension_os "${extension_os}/Ubuntu${ubuntu_vers}")
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
            -Wabi -Wpacked
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


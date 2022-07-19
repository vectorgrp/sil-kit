# Disable warnings in ThirdParty folder
add_compile_options(-w)

function(addTestProject)
    # Prevent overriding the parent project's compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # HACK: Temporarily remove the project version to avoid a warning related to unversioned project definitions
    set(PROJECT_VERSION)
    set(PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MINOR)
    set(PROJECT_VERSION_PATCH)

    # Google mock
    add_compile_definitions(UNIT_TEST)
    if(MSVC)
        # Ensure that std::tuple is used instead of std::tr1::tuple
        add_compile_definitions(
            GTEST_LANG_CXX11=1
            _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING=1
        )
    endif()

    set(_gtest_version 1.11.0)
    message(STATUS "Add subdirectory ThirdParty/googletest-${_gtest_version}")
    add_subdirectory(googletest-${_gtest_version}
        googletest-${_gtest_version}
        EXCLUDE_FROM_ALL
    )

    if(MSVC)
        # Ensure that compile definitions are propagated to gmock users
        target_compile_definitions(gmock
            PUBLIC
            GTEST_LANG_CXX11=1
            _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING=1
        )
        target_compile_definitions(gtest
            PUBLIC
            GTEST_LANG_CXX11=1
            _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING=1
        )
    endif()
    set_property(TARGET gmock_main PROPERTY FOLDER "ThirdParty")
    set_property(TARGET gtest PROPERTY FOLDER "ThirdParty")
endfunction()

if(SILKIT_BUILD_TESTS)
    addTestProject()
endif()

# NB: we use a standalone, bundled ASIO now.
# the binary distribution lacks the Asio sources
# but it should be included/deployed by our packaging tools
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${CMAKE_CURRENT_LIST_DIR}/asio/asio/include)
set(ASIO_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/asio/asio/include)

if(NOT EXISTS ${ASIO_INCLUDE_DIR})
    message(FATAL_ERROR "Asio include directory not found: ${ASIO_INCLUDE_DIR}")
else()
    message(STATUS "Asio include directory found:${ASIO_INCLUDE_DIR}")
endif()

# Third party YAML parsing 
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Disable because of gmock import conflict" FORCE)
add_subdirectory(yaml-cpp
	yaml-cpp
	EXCLUDE_FROM_ALL
	)


# libfmt
function(include_fmt)
    # function() adds an additional scope
    # XXX fmt is in our public ILogger.hpp, and also in the interface link list of SilKitInterface
    # therefore it *must* be install-exported, otherwise the install-export of SIL Kit fails
    set(FMT_VERSION "6.1.0")
    set(FMT_INSTALL ON CACHE BOOL "Make sure the fmt-header-only interface target is exported" FORCE)
    message(STATUS "Add subdirectory ${CMAKE_CURRENT_LIST_DIR}/fmt-${FMT_VERSION}")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/fmt-${FMT_VERSION}
        ${CMAKE_BINARY_DIR}/ThirdParty/fmt-${FMT_VERSION}
        EXCLUDE_FROM_ALL
    )
endfunction()
include_fmt()

# spdlog
function(include_spdlog)
    # function() adds an additional scope
    # Temporarily patch some CMake variables to trick spdlog's install commands into using our install-folder layout
    set(CMAKE_INSTALL_LIBDIR "${INSTALL_LIB_DIR}")
    set(CMAKE_INSTALL_INCLUDEDIR  "${INSTALL_INCLUDE_DIR}")
    set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    message(STATUS "Add subdirectory ThirdParty/spdlog")
    add_subdirectory(
        spdlog
        ${CMAKE_BINARY_DIR}/ThirdParty/spdlog
        EXCLUDE_FROM_ALL
    )
endfunction()

include_spdlog()

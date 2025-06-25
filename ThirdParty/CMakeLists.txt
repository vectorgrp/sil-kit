# Disable warnings in ThirdParty folder


set(SILKIT_THIRD_PARTY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "" FORCE)
set(SILKIT_THIRD_PARTY_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE STRING "" FORCE)


function(include_gtest)
    if(NOT SILKIT_USE_SYSTEM_LIBRARIES)
        silkit_clean_default_compileflags()

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

        # disable the IPO for GTest due to CMake policy CMP0069 changes
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
        endif()

        #check the submodule tag for the current version.
        message(STATUS "Add subdirectory ThirdParty/googletest")
        add_subdirectory(
            "${SILKIT_THIRD_PARTY_SOURCE_DIR}/googletest"
            "${SILKIT_THIRD_PARTY_BINARY_DIR}/_tp_googletest"
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
            silkit_target_clean_compileflags(gtest)
            silkit_target_clean_compileflags(gmock_main)
        endif()
        set_property(TARGET gmock_main PROPERTY FOLDER "ThirdParty")
        set_property(TARGET gtest PROPERTY FOLDER "ThirdParty")
    else()
        find_package(GTest REQUIRED)
        add_library(gtest ALIAS GTest::gtest)
        add_library(gmock_main ALIAS GTest::gmock_main)
    endif()
endfunction()


# NB: we use a standalone, bundled ASIO now.
# the binary distribution lacks the Asio sources
# but it should be included/deployed by our packaging tools
function(include_asio)

    if( SILKIT_USE_SYSTEM_LIBRARIES)
        find_path(ASIO_INCLUDE_DIR asio.hpp)
    else()
        set(ASIO_INCLUDE_DIR ${SILKIT_THIRD_PARTY_SOURCE_DIR}/asio/asio/include)
    endif()

    if(NOT EXISTS ${ASIO_INCLUDE_DIR})
        message(FATAL_ERROR "Asio include directory not found: ${ASIO_INCLUDE_DIR}")
    else()
        message(STATUS "Asio include directory found:${ASIO_INCLUDE_DIR}")
    endif()

    silkit_clean_default_compileflags()
    add_library(asio INTERFACE)
    target_include_directories(asio INTERFACE "${ASIO_INCLUDE_DIR}")
    set(SILKIT_THIRD_PARTY_ASIO "asio" CACHE STRING "Name of the third party asio target")
endfunction()

# libfmt
function(include_fmt)
    if(NOT SILKIT_USE_SYSTEM_LIBRARIES)
        # function() adds an additional scope
        #set(FMT_INSTALL ON CACHE BOOL "Make sure the fmt-header-only interface target is exported" FORCE)
        silkit_clean_default_compileflags()

        message(STATUS "Add subdirectory ${CMAKE_CURRENT_LIST_DIR}/fmt")
        add_subdirectory(
            "${SILKIT_THIRD_PARTY_SOURCE_DIR}/fmt"
            "${SILKIT_THIRD_PARTY_BINARY_DIR}/_tp_fmt"
            EXCLUDE_FROM_ALL
        )
        if(TARGET fmt-header-only)
            set(SILKIT_THIRD_PARTY_FMT_HEADER_ONLY_TARGET "fmt-header-only" CACHE STRING "Header only target for third party libfmt")
        else()
            set(SILKIT_THIRD_PARTY_FMT_HEADER_ONLY_TARGET "" CACHE STRING "Header only target for third party libfmt")
        endif()
    else()
        # Find system libfmt
        include(FindPkgConfig)
        pkg_check_modules(fmt REQUIRED IMPORTED_TARGET GLOBAL fmt)
        add_library(fmt-header-only INTERFACE)
        target_include_directories(fmt-header-only INTERFACE ${fmt_INCLUDE_DIRS})
        target_compile_definitions(fmt-header-only INTERFACE FMT_HEADER_ONLY=1)

        add_library(fmt::fmt-header-only ALIAS fmt-header-only)
        add_library(fmt::fmt ALIAS fmt-header-only)
        set(SPDLOG_FMT_EXTERNAL Off CACHE BOOL "we have our own")
        set(SPDLOG_FMT_EXTERNAL_HO Off CACHE BOOL "we have our own")
        set(SILKIT_THIRD_PARTY_FMT_HEADER_ONLY_TARGET "fmt::fmt-header-only" CACHE STRING "Header only target for third party libfmt (system library)")
    endif()
endfunction()

# spdlog
function(include_spdlog)
    if(NOT SILKIT_USE_SYSTEM_LIBRARIES)
        # function() adds an additional scope
        # Temporarily patch some CMake variables to trick spdlog's install commands into using our install-folder layout

        silkit_clean_default_compileflags()

        set(CMAKE_INSTALL_LIBDIR "${INSTALL_LIB_DIR}")
        set(CMAKE_INSTALL_INCLUDEDIR  "${INSTALL_INCLUDE_DIR}")
        set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
        set(CMAKE_CXX_VISIBILITY_PRESET hidden)
        message(STATUS "Add subdirectory ThirdParty/spdlog")
        set(SPDLOG_FMT_EXTERNAL_HO ON)
        add_subdirectory(
            "${SILKIT_THIRD_PARTY_SOURCE_DIR}/spdlog"
            "${SILKIT_THIRD_PARTY_BINARY_DIR}/_tp_spdlog"
            EXCLUDE_FROM_ALL
        )
    else()
        # Find spdlog
        find_package(spdlog REQUIRED)
    endif()
endfunction()


function(include_oatpp)
    silkit_clean_default_compileflags()

    set(OATPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(OATPP_INSTALL OFF CACHE BOOL "" FORCE)
    set(OATPP_ADD_LINK_LIBS OFF CACHE BOOL "" FORCE)
    set(OATPP_LINK_ATOMIC OFF CACHE BOOL "" FORCE)

    set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_C_VISIBILITY_PRESET hidden)
    # work around old cmake in oatpp:
    set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)

    add_subdirectory(
        "${SILKIT_THIRD_PARTY_SOURCE_DIR}/oatpp"
        "${SILKIT_THIRD_PARTY_BINARY_DIR}/_tp_oatpp"
        EXCLUDE_FROM_ALL
    )
    silkit_target_clean_compileflags(oatpp)

    if (CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        target_compile_options(oatpp PRIVATE "/wd4244")
    endif ()

    set_property(TARGET oatpp PROPERTY CXX_VISIBILITY_PRESET hidden)
    set_property(TARGET oatpp PROPERTY VISIBILITY_INLINES_HIDDEN ON)
endfunction()

function(include_rapidyaml)
		add_subdirectory(
            "${SILKIT_THIRD_PARTY_SOURCE_DIR}/rapidyaml"
            "${SILKIT_THIRD_PARTY_BINARY_DIR}/_tp_rapidyaml"
    )
    add_library(rapidyaml ALIAS rapidyaml-static)
endfunction()

function(silkit_add_third_party_packages)
    if(SILKIT_BUILD_TESTS)
        include_gtest()
    endif()

    include_asio()
    include_fmt()
    include_spdlog()
    include_rapidyaml()

    if (SILKIT_BUILD_DASHBOARD)
        include_oatpp()
    endif ()
endfunction()



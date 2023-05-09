# Disable warnings in ThirdParty folder

function(include_gtest)
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
    add_subdirectory(googletest
        googletest
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
endfunction()

if(SILKIT_BUILD_TESTS)
    include_gtest()
endif()

# NB: we use a standalone, bundled ASIO now.
# the binary distribution lacks the Asio sources
# but it should be included/deployed by our packaging tools
function(include_asio)
    silkit_clean_default_compileflags()
    add_compile_options(-w)
    add_library(asio INTERFACE)
    target_include_directories(asio INTERFACE ${CMAKE_CURRENT_LIST_DIR}/asio/asio/include)
    set(ASIO_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/asio/asio/include)

    if(NOT EXISTS ${ASIO_INCLUDE_DIR})
        message(FATAL_ERROR "Asio include directory not found: ${ASIO_INCLUDE_DIR}")
    else()
        message(STATUS "Asio include directory found:${ASIO_INCLUDE_DIR}")
    endif()
endfunction()
include_asio()

# Third party YAML parsing 
function(include_yamlcpp)
    set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Disable because of gmock import conflict" FORCE)
    silkit_clean_default_compileflags()
    add_compile_options(-w)

    # disable the IPO for GTest due to CMake policy CMP0069 changes
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
    endif()

    add_subdirectory(yaml-cpp
        yaml-cpp
        EXCLUDE_FROM_ALL
	)
    # yaml-cpp explicitly sets /W3
    silkit_target_clean_compileflags(yaml-cpp)

    add_library(YamlInterface INTERFACE)
    target_include_directories(YamlInterface
        INTERFACE $<TARGET_PROPERTY:yaml-cpp,INTERFACE_INCLUDE_DIRECTORIES>)

endfunction()
include_yamlcpp()

# libfmt
function(include_fmt)
    # function() adds an additional scope
    #set(FMT_INSTALL ON CACHE BOOL "Make sure the fmt-header-only interface target is exported" FORCE)
    silkit_clean_default_compileflags()
    add_compile_options(-w)

    message(STATUS "Add subdirectory ${CMAKE_CURRENT_LIST_DIR}/fmt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/fmt
        ${CMAKE_BINARY_DIR}/ThirdParty/fmt
        EXCLUDE_FROM_ALL
    )
endfunction()
include_fmt()

# spdlog
function(include_spdlog)
    # function() adds an additional scope
    # Temporarily patch some CMake variables to trick spdlog's install commands into using our install-folder layout

    silkit_clean_default_compileflags()
    add_compile_options(-w)

    set(CMAKE_INSTALL_LIBDIR "${INSTALL_LIB_DIR}")
    set(CMAKE_INSTALL_INCLUDEDIR  "${INSTALL_INCLUDE_DIR}")
    set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    message(STATUS "Add subdirectory ThirdParty/spdlog")
    set(SPDLOG_FMT_EXTERNAL_HO ON)
    add_subdirectory(
        spdlog
        ${CMAKE_BINARY_DIR}/ThirdParty/spdlog
        EXCLUDE_FROM_ALL
    )
endfunction()

include_spdlog()


if(SILKIT_BUILD_DASHBOARD)
    function(include_oatpp)
        silkit_clean_default_compileflags()
        add_compile_options(-w)

        set(OATPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(OATPP_INSTALL OFF CACHE BOOL "" FORCE)
        set(OATPP_ADD_LINK_LIBS OFF CACHE BOOL "" FORCE)
        set(OATPP_LINK_ATOMIC OFF CACHE BOOL "" FORCE)

        set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
        set(CMAKE_CXX_VISIBILITY_PRESET hidden)
        set(CMAKE_C_VISIBILITY_PRESET hidden)
        # work around old cmake in oatpp:
        set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)
        add_subdirectory(oatpp)
        silkit_target_clean_compileflags(oatpp)
        set_property(TARGET oatpp PROPERTY CXX_VISIBILITY_PRESET hidden)
        set_property(TARGET oatpp PROPERTY VISIBILITY_INLINES_HIDDEN ON)
    endfunction()

    include_oatpp()
endif()

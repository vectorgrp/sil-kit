# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# SilKitYaml -- self-contained, header-only YAML (de)serialization on top of
# rapidyaml. Used internally by SIL Kit and vendorable into other projects via
# subrepo/submodule (add_subdirectory). It is NOT installed or exported, and its
# symbols are not leaked into the SIL Kit shared library.
#
# Requirement: the host project must provide a `rapidyaml` target.

if(NOT TARGET rapidyaml)
    message(FATAL_ERROR
        "SilKitYaml requires a 'rapidyaml' target. Provide one before add_subdirectory(SilKitYaml).")
endif()

add_library(SilKitYaml INTERFACE)
add_library(SilKitYaml::SilKitYaml ALIAS SilKitYaml)

target_include_directories(SilKitYaml
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# rapidyaml carries its include dirs and RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS=1.
target_link_libraries(SilKitYaml INTERFACE rapidyaml)
target_compile_features(SilKitYaml INTERFACE cxx_std_17)

# Convenience project for IDEs to surface the headers.
file(GLOB_RECURSE SILKIT_YAML_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
add_custom_target(SilKitYamlApi SOURCES ${SILKIT_YAML_HEADERS})

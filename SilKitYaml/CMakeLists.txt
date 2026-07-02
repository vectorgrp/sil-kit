# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# SilKitYaml -- self-contained, header-only YAML (de)serialization on top of
# rapidyaml. Used internally by SIL Kit and vendorable into other projects via
# subrepo/submodule (add_subdirectory). It is NOT installed or exported, and its
# symbols are not leaked into the SIL Kit shared library.
#
# rapidyaml dependency: if the host project already provides a `rapidyaml` target
# (as SIL Kit does in-tree), it is used as-is. Otherwise SilKitYaml provisions the
# vendored rapidyaml itself, locating it via SILKIT_TOP_DIR (the root of the SIL
# Kit tree). Both SILKIT_TOP_DIR and the concrete SILKITYAML_RAPIDYAML_DIR are
# overridable so hosts with a different layout, or their own rapidyaml, still work.

# Root of the SIL Kit tree. Honor an outer definition (set by the top-level
# CMakeLists in-tree); otherwise self-locate: SilKitYaml lives at <root>/SilKitYaml.
if(NOT DEFINED SILKIT_TOP_DIR)
    get_filename_component(SILKIT_TOP_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()

set(SILKITYAML_RAPIDYAML_DIR "${SILKIT_TOP_DIR}/ThirdParty/rapidyaml"
    CACHE PATH "Location of the rapidyaml sources consumed by SilKitYaml")

if(NOT TARGET rapidyaml)
    if(EXISTS "${SILKITYAML_RAPIDYAML_DIR}/CMakeLists.txt")
        add_subdirectory("${SILKITYAML_RAPIDYAML_DIR}"
                         "${CMAKE_CURRENT_BINARY_DIR}/_rapidyaml")
        # The rapidyaml folder only defines `rapidyaml-static`; provide the alias.
        if(NOT TARGET rapidyaml)
            add_library(rapidyaml ALIAS rapidyaml-static)
        endif()
    else()
        message(FATAL_ERROR
            "SilKitYaml: no 'rapidyaml' target and none found at "
            "'${SILKITYAML_RAPIDYAML_DIR}'. Provide a 'rapidyaml' target, or set "
            "SILKIT_TOP_DIR / SILKITYAML_RAPIDYAML_DIR to the rapidyaml location.")
    endif()
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

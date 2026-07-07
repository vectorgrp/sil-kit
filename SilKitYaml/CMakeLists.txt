# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# SilKitYaml -- self-contained, header-only YAML (de)serialization on top of
# rapidyaml. Used internally by SIL Kit and vendorable into other projects via
# subrepo/submodule (add_subdirectory). It is NOT installed or exported, and its
# symbols are not leaked into the SIL Kit shared library.

if (NOT DEFINED SILKITYAML_RAPIDYAML_TARGET)
    # If the entire SIL Kit source tree is added as a subdirectory or built on its own,
    # the rapidyaml target is provided in the 'SILKIT_THIRD_PARTY_RAPIDYAML_TARGET' variable.

    if (TARGET "${SILKIT_THIRD_PARTY_RAPIDYAML_TARGET}")
        set(SILKITYAML_RAPIDYAML_TARGET "${SILKIT_THIRD_PARTY_RAPIDYAML_TARGET}")
        message(NOTICE "SilKitYaml: Using rapidyaml target defined by SIL Kit")
    else ()
        # If SilKitYaml is added as a subdirectory directly (i.e., without SIL Kit),
        # check if rapidyaml is available from the surrounding SIL Kit source tree.

        get_filename_component(
            SILKIT_THIRD_PARTY_RAPIDYAML_DIR
            "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/rapidyaml"
            REALPATH
        )

        if (EXISTS "${SILKIT_THIRD_PARTY_RAPIDYAML_DIR}/CMakeLists.txt")
            add_subdirectory("${SILKIT_THIRD_PARTY_RAPIDYAML_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/_tp_rapidyaml")
            set(SILKITYAML_RAPIDYAML_TARGET rapidyaml)
        endif ()
    endif ()
endif ()

if (NOT TARGET "${SILKITYAML_RAPIDYAML_TARGET}")
    message(FATAL_ERROR "SilKitYaml: Failed to find the required rapidyaml dependency. Please provide a suitable target as 'SILKITYAML_RAPIDYAML_TARGET'.")
endif ()

add_library(SilKitYaml INTERFACE)
add_library(SilKitYaml::SilKitYaml ALIAS SilKitYaml)

target_include_directories(SilKitYaml
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# rapidyaml carries its include dirs and RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS=1.
target_link_libraries(SilKitYaml INTERFACE "${SILKITYAML_RAPIDYAML_TARGET}")
target_compile_features(SilKitYaml INTERFACE cxx_std_17)

# Convenience project for IDEs to surface the headers.
file(GLOB_RECURSE SILKIT_YAML_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
add_custom_target(SilKitYamlApi SOURCES ${SILKIT_YAML_HEADERS})

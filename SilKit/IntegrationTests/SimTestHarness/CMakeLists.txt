# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# ==============================================================================
#  SimTestHarness
# ==============================================================================

add_library(S_ITests_STH STATIC
    SimSystemController.hpp
    SimTestHarness.hpp
    SimTestHarness.cpp
)

target_link_libraries(S_ITests_STH
    PUBLIC SilKit
)

target_include_directories(S_ITests_STH
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

set_property(TARGET S_ITests_STH PROPERTY FOLDER "Tests")

set_target_properties(S_ITests_STH PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

# ==============================================================================
#  SimTestHarness using the "Legacy C++ API" without the C-API layer for direct
#  access to the internal symbols. This is required for some ITests.
# ==============================================================================

add_library(S_ITests_STH_Internals STATIC
    SimSystemController.hpp
    SimTestHarness.hpp
    SimTestHarness.cpp
)

target_link_libraries(S_ITests_STH_Internals
    PUBLIC S_SilKitImpl
)

target_include_directories(S_ITests_STH_Internals
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(S_ITests_STH_Internals
    PUBLIC SILKIT_LEGACY_CPP_API=1
)

set_property(TARGET S_ITests_STH_Internals PROPERTY FOLDER "Tests")

set_target_properties(S_ITests_STH_Internals
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

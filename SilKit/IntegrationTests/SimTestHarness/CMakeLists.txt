# Copyright (c) 2022 Vector Informatik GmbH
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


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

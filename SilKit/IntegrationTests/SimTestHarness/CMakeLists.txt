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

add_library(S_IntegrationTests_SimTestHarness STATIC
    SimSystemController.hpp
    SimTestHarness.hpp
    SimTestHarness.cpp
)
target_link_libraries(S_IntegrationTests_SimTestHarness
    PRIVATE I_SilKit_Core_Internal

    PUBLIC SilKit
    PUBLIC I_SilKit_Config_TestUtils
)
target_include_directories(S_IntegrationTests_SimTestHarness
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)
set_property(TARGET S_IntegrationTests_SimTestHarness PROPERTY FOLDER "Tests")

set_target_properties(S_IntegrationTests_SimTestHarness PROPERTIES
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

# ==============================================================================
# Hourglass-Variant
# ==============================================================================

add_library(S_IntegrationTests_SimTestHarness_Hourglass STATIC
    SimSystemController.hpp
    SimTestHarness.hpp
    SimTestHarness.cpp
)

target_link_libraries(S_IntegrationTests_SimTestHarness_Hourglass
    PRIVATE I_SilKit_Config_TestUtils

    PUBLIC SilKit
    PUBLIC SilKit_Hourglass
)

target_include_directories(S_IntegrationTests_SimTestHarness_Hourglass

    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET S_IntegrationTests_SimTestHarness_Hourglass PROPERTY FOLDER "Tests")

set_target_properties(S_IntegrationTests_SimTestHarness_Hourglass
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

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

add_library(DummyExtension SHARED
    DummyExtension.cpp
    DummyExtension.hpp
)
target_link_libraries(DummyExtension PRIVATE O_SilKit_Extensions O_SilKit_VersionImpl SilKit)
target_compile_definitions(DummyExtension PRIVATE SILKIT_EXTENSION_OS="Dummy")
set_property(TARGET DummyExtension PROPERTY FOLDER "Tests")
set_target_properties(DummyExtension PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/silkit_library_test"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/silkit_library_test"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/silkit_library_test"
)

add_library(WrongVersionExtension SHARED WrongVersionExtension.cpp)
target_link_libraries(WrongVersionExtension PRIVATE O_SilKit_Extensions O_SilKit_VersionImpl SilKit)
set_property(TARGET WrongVersionExtension PROPERTY FOLDER "Tests")
target_compile_definitions(WrongVersionExtension PRIVATE SILKIT_EXTENSION_OS="WrongVersion")
set_target_properties(WrongVersionExtension PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_library(WrongBuildSystem  SHARED WrongBuildSystem.cpp) 
target_link_libraries(WrongBuildSystem PRIVATE O_SilKit_Extensions O_SilKit_VersionImpl SilKit)
target_compile_definitions(WrongBuildSystem PRIVATE SILKIT_EXTENSION_OS="WrongBuildSystem")
set_property(TARGET WrongBuildSystem PROPERTY FOLDER "Tests")
set_target_properties(WrongBuildSystem PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_silkit_test(Test_SilKitExtensions SOURCES Test_SilKitExtensions.cpp LIBS O_SilKit_Extensions O_SilKit_VersionImpl O_SilKit_Util_Filesystem I_SilKit_Core_Mock_Participant SilKit)

if(${SILKIT_BUILD_TESTS})
    add_dependencies(Test_SilKitExtensions
        DummyExtension
        WrongVersionExtension
        WrongBuildSystem
        )
endif()


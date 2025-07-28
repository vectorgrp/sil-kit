# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

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

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_SilKitExtensions.cpp LIBS O_SilKit_Extensions O_SilKit_VersionImpl I_SilKit_Core_Mock_Participant SilKit)

if(${SILKIT_BUILD_TESTS})
    add_dependencies(SilKitUnitTests
        DummyExtension
        WrongVersionExtension
        WrongBuildSystem
    )
endif()


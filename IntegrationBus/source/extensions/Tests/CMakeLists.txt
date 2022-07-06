# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(DummyExtension SHARED
    DummyExtension.cpp
    DummyExtension.hpp
)
target_link_libraries(DummyExtension PRIVATE SilKitExtensions )
set_property(TARGET DummyExtension PROPERTY FOLDER "Tests")
set_target_properties(DummyExtension PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/silkit_library_test"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/silkit_library_test"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/silkit_library_test"
)

add_library(WrongVersionExtension SHARED WrongVersionExtension.cpp)
target_link_libraries(WrongVersionExtension PRIVATE SilKitExtensions )
set_property(TARGET WrongVersionExtension PROPERTY FOLDER "Tests")
set_target_properties(WrongVersionExtension PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_library(WrongBuildSystem  SHARED WrongBuildSystem.cpp) 
target_link_libraries(WrongBuildSystem PRIVATE SilKitExtensions )
set_property(TARGET WrongBuildSystem PROPERTY FOLDER "Tests")
set_target_properties(WrongBuildSystem PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_silkit_test(TestSilKitExtensions SOURCES SilKitExtensionsTest.cpp LIBS SilKitExtensions SilKitFilesystem SilKitMwMockParticipant)

if(${SILKIT_BUILD_TESTS})
    add_dependencies(TestSilKitExtensions
        DummyExtension
        WrongVersionExtension
        WrongBuildSystem
        )
endif()


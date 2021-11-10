# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(DummyExtension SHARED
    DummyExtension.cpp
    DummyExtension.hpp
)
target_link_libraries(DummyExtension PRIVATE IbExtensions )
set_property(TARGET DummyExtension PROPERTY FOLDER "Tests")

add_library(WrongVersionExtension SHARED WrongVersionExtension.cpp)
target_link_libraries(WrongVersionExtension PRIVATE IbExtensions )
set_property(TARGET WrongVersionExtension PROPERTY FOLDER "Tests")

add_library(WrongBuildSystem  SHARED WrongBuildSystem.cpp) 
target_link_libraries(WrongBuildSystem PRIVATE IbExtensions )
set_property(TARGET WrongBuildSystem PROPERTY FOLDER "Tests")

add_vib_test(TestIbExtensions SOURCES IbExtensionsTest.cpp LIBS IbExtensions)

add_dependencies(TestIbExtensions
    DummyExtension
    WrongVersionExtension
    WrongBuildSystem
    )


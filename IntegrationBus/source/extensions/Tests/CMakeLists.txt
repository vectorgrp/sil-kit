add_library(DummyExtension SHARED
    DummyExtension.cpp
    DummyExtension.hpp
)
target_link_libraries(DummyExtension PRIVATE IbExtensions )

add_library(WrongVersionExtension SHARED WrongVersionExtension.cpp)
target_link_libraries(WrongVersionExtension PRIVATE IbExtensions )

add_library(WrongBuildSystem  SHARED WrongBuildSystem.cpp) 
target_link_libraries(WrongBuildSystem PRIVATE IbExtensions )

add_vib_test(TestIbExtensions SOURCES IbExtensionsTest.cpp LIBS IbExtensions)

add_dependencies(TestIbExtensions
    DummyExtension
    WrongVersionExtension
    WrongBuildSystem
    )

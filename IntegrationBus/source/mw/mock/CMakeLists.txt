add_library(IbMwMock STATIC
    MockComAdapter.hpp
    NullConnectionComAdapter.cpp
    NullConnectionComAdapter.hpp
)

target_include_directories(IbMwMock
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(IbMwMock
    IbMwComAdapter
)

if (MSVC)
    target_compile_options(IbMwMock PRIVATE "/bigobj")
endif (MSVC)


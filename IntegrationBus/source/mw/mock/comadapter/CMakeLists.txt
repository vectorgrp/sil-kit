add_library(IbMwMockComAdapter INTERFACE
)

target_include_directories(IbMwMockComAdapter
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(IbMwMockComAdapter
    INTERFACE IbMwLogging
)

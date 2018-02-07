add_library(IbUtil INTERFACE)

target_sources(IbUtil INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/tuple_tools/bind.hpp
    ${CMAKE_CURRENT_LIST_DIR}/tuple_tools/conditional.hpp
    ${CMAKE_CURRENT_LIST_DIR}/tuple_tools/for_each.hpp
    ${CMAKE_CURRENT_LIST_DIR}/tuple_tools/predicative_get.hpp
)

target_include_directories(IbUtil
    INTERFACE .
)



add_subdirectory(tests)


find_package(Threads REQUIRED)

add_library(IbMwRegistry STATIC
    Registry.hpp
    Registry.cpp
)

target_include_directories(IbMwRegistry
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

if (MSVC)
    target_compile_options(IbMwRegistry PRIVATE "/bigobj")
endif (MSVC)

target_link_libraries(IbMwRegistry
    PUBLIC
    IbMwVAsio
    IbConfig
    Threads::Threads
)

add_executable(IbMwRegistryDemo
    RegistryDemo.cpp
)

target_link_libraries(IbMwRegistryDemo
    IbMwRegistry
)

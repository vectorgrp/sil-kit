find_package(Threads REQUIRED)

add_executable(IbMwRegistry
    Registry.hpp
    Registry.cpp
    ../vasio/VAsioConnection.hpp
)

target_link_libraries(IbMwRegistry
    IbMwVAsio
    IbConfig
    Threads::Threads
)

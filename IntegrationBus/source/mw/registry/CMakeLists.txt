add_executable(IbMwRegistry
    Registry.hpp
    Registry.cpp
    ../vasio/VAsioConnection.hpp
)

target_link_libraries(IbMwRegistry
    PRIVATE IbMwVAsio
    PRIVATE IbConfig
)
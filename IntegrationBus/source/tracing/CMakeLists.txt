add_library(IbTracing STATIC
    PcapSink.cpp
    PcapSink.hpp

    detail/NamedPipe.hpp

    Tracing.hpp
    Tracing.cpp

    #public interface used for trace sink implementations
    ${CMAKE_CURRENT_LIST_DIR}/../../include/ib/extensions/ITraceMessageSink.hpp
)

target_include_directories(IbTracing
    PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}
        detail
        #we need access to the simulation datatypes, without producing cyclic dependencies:
        ${CMAKE_CURRENT_LIST_DIR}/../sim/can
        ${CMAKE_CURRENT_LIST_DIR}/../sim/eth
        ${CMAKE_CURRENT_LIST_DIR}/../sim/fr
        ${CMAKE_CURRENT_LIST_DIR}/../sim/generic
        ${CMAKE_CURRENT_LIST_DIR}/../sim/io
        ${CMAKE_CURRENT_LIST_DIR}/../sim/lin
    PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(IbTracing
    PRIVATE IbInterface
    PRIVATE IbMwLogging
)

if(WIN32)
    target_sources(IbTracing PRIVATE
        detail/NamedPipeWin.hpp
        detail/NamedPipeWin.cpp
        )
elseif(UNIX)
    target_sources(IbTracing PRIVATE
        detail/NamedPipeLinux.hpp
        detail/NamedPipeLinux.cpp
        )
else()
    message(FATAL_ERROR "ERROR: unsupported platform for NamedPipe!")
endif()


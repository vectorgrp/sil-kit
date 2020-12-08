######################################################################
# VIB Tracing and Replay component
# - Some tracing and replay functionality is implemented in 
#   the external vibe-mdf4tracing extension.
######################################################################

add_library(IbTracing STATIC
    PcapSink.cpp
    PcapSink.hpp

    PcapReader.cpp
    PcapReader.hpp

    detail/NamedPipe.hpp

    Tracing.hpp
    Tracing.cpp

    #public interface used for trace sink implementations
    ${CMAKE_CURRENT_LIST_DIR}/../../include/ib/extensions/ITraceMessageSink.hpp
    ${CMAKE_CURRENT_LIST_DIR}/../../include/ib/extensions/TraceMessage.hpp

    #Trace Replaying utilities
    IReplayDataController.hpp

    PcapReplay.cpp
    PcapReplay.hpp

    ReplayScheduler.hpp
    ReplayScheduler.cpp
    
    ReplayController.hpp
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
        ${CMAKE_CURRENT_LIST_DIR}/mock/
)
target_link_libraries(IbTracing
    PRIVATE IbInterface
            IbMwLogging
            IbExtensions
    INTERFACE #because we pull private headers into our ReplayController.hpp, FIXME
            IbSimEthernet
            IbSimCan
            IbSimFlexRay
            IbSimLin
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

#ib_enable_warnings(IbTracing)


add_vib_test(TestTracing_Replay SOURCES ReplayTest.cpp LIBS IbMwMockComAdapter IbTracing )
add_vib_test(TestTracing_Pcap SOURCES PcapTest.cpp)


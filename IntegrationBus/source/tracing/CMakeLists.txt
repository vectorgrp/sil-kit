# Copyright (c) Vector Informatik GmbH. All rights reserved.
######################################################################
# VIB Tracing and Replay component
# - Some tracing and replay functionality is implemented in
#   the external vibe-mdf4tracing extension.
######################################################################

option(IB_BUILD_COMPONENT_TRACING "Temporarily disable building of Tracing component" OFF)
if(IB_BUILD_COMPONENT_TRACING)
    add_library(IbTracing STATIC
        PcapSink.cpp
        PcapSink.hpp

        PcapReader.cpp
        PcapReader.hpp

        detail/NamedPipe.hpp

        Tracing.hpp
        Tracing.cpp

        ITraceMessageSink.hpp
        ITraceMessageSource.hpp
        TraceMessage.hpp

        #Trace Replaying utilities
        IReplayDataController.hpp

        PcapReplay.cpp
        PcapReplay.hpp

        ReplayScheduler.hpp
        ReplayScheduler.cpp
    )

    target_include_directories(IbTracing
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            detail
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/mock/
    )
    target_link_libraries(IbTracing
        PRIVATE
            IbInterface
            IbMwLogging
        PUBLIC
            IbInternal
            IbSimEthernet
            IbSimCan
            IbSimFlexray
            IbSimLin
            IbSimDataMessage
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

    add_vib_test(TestTracing_Replay SOURCES ReplayTest.cpp LIBS IbMwMockParticipant IbTracing )
    add_vib_test(TestTracing_Pcap SOURCES PcapTest.cpp LIBS IbTracing IbMwMockParticipant )
else()
    # At least provide some mocks for testing
    add_library(IbTracing INTERFACE)
    target_include_directories(IbTracing
        INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/mock/
    )
    target_link_libraries(IbTracing
        INTERFACE
            IbInternal
            IbSimEthernet
    )
endif()

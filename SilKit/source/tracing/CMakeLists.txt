# Copyright (c) Vector Informatik GmbH. All rights reserved.
######################################################################
# SILKIT Tracing and Replay component
# - Some tracing and replay functionality is implemented in
#   the external silkite-mdf4tracing extension.
######################################################################

option(SILKIT_BUILD_COMPONENT_TRACING "Temporarily disable building of Tracing component" OFF)
if(SILKIT_BUILD_COMPONENT_TRACING)
    add_library(SilKitTracing STATIC
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

    target_include_directories(SilKitTracing
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            detail
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/mock/
    )
    target_link_libraries(SilKitTracing
        PRIVATE
            SilKitInterface
            SilKitMwLogging
        PUBLIC
            SilKitInternal
            SilKitSimEthernet
            SilKitSimCan
            SilKitSimFlexray
            SilKitSimLin
            SilKitSimDataMessage
    )

    if(WIN32)
        target_sources(SilKitTracing PRIVATE
            detail/NamedPipeWin.hpp
            detail/NamedPipeWin.cpp
            )
    elseif(UNIX)
        target_sources(SilKitTracing PRIVATE
            detail/NamedPipeLinux.hpp
            detail/NamedPipeLinux.cpp
            )
    else()
        message(FATAL_ERROR "ERROR: unsupported platform for NamedPipe!")
    endif()

    add_silkit_test(TestTracing_Replay SOURCES ReplayTest.cpp LIBS SilKitMwMockParticipant SilKitTracing )
    add_silkit_test(TestTracing_Pcap SOURCES PcapTest.cpp LIBS SilKitTracing SilKitMwMockParticipant )
else()
    # At least provide some mocks for testing
    add_library(SilKitTracing INTERFACE)
    target_include_directories(SilKitTracing
        INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/mock/
    )
    target_link_libraries(SilKitTracing
        INTERFACE
            SilKitInternal
            SilKitSimEthernet
    )
endif()

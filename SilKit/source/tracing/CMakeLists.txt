# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

######################################################################
# SIL Kit Tracing and Replay component
# - Some tracing and replay functionality is implemented in
#   the external sil-kit-mdf4tracing extension.
######################################################################

add_library(I_SilKit_Tracing INTERFACE)

target_include_directories(I_SilKit_Tracing
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/mock/
)
target_link_libraries(I_SilKit_Tracing
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Services_Ethernet
    INTERFACE I_SilKit_Services_Can
    INTERFACE I_SilKit_Services_Flexray
    INTERFACE I_SilKit_Services_Lin
    INTERFACE I_SilKit_Services_PubSub
    INTERFACE I_SilKit_Extensions
)

add_library(O_SilKit_Tracing OBJECT
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

target_include_directories(O_SilKit_Tracing
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        detail
)
target_link_libraries(O_SilKit_Tracing
    PUBLIC I_SilKit_Tracing

    PRIVATE I_SilKit_Services_Logging
)

if(WIN32)
    target_sources(O_SilKit_Tracing PRIVATE
        detail/NamedPipeWin.hpp
        detail/NamedPipeWin.cpp
        )
elseif(UNIX)
    target_sources(O_SilKit_Tracing PRIVATE
        detail/NamedPipeLinux.hpp
        detail/NamedPipeLinux.cpp
        )
else()
    message(FATAL_ERROR "ERROR: unsupported platform for NamedPipe!")
endif()

#XXX not viable, yet: add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_Replay.cpp LIBS I_SilKit_Core_Mock_Participant O_SilKit_Tracing )
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_Pcap.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_EthernetReplay.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant S_SilKitImpl)


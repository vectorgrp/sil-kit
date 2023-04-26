# Copyright (c) 2022 Vector Informatik GmbH
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

######################################################################
# SILKIT Tracing and Replay component
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

#XXX not viable, yet: add_silkit_test(Test_Replay SOURCES Test_Replay.cpp LIBS I_SilKit_Core_Mock_Participant O_SilKit_Tracing )
add_silkit_test(Test_Pcap SOURCES Test_Pcap.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)

add_silkit_test(Test_EthernetReplay SOURCES Test_EthernetReplay.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant S_SilKitImpl)

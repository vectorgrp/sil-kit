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

find_package(Threads REQUIRED)


add_library(I_SilKit_Core_VAsio INTERFACE)

target_include_directories(I_SilKit_Core_VAsio
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(I_SilKit_Core_VAsio
    INTERFACE asio
)


add_library(S_SilKit_Core_VAsio STATIC
    VAsioConnection.hpp
    VAsioConnection.cpp

    VAsioProtocolVersion.hpp
    VAsioSerdes.hpp
    VAsioSerdes.cpp
    VAsioSerdes_Protocol30.hpp
    VAsioSerdes_Protocol30.cpp

    SilKitLink.hpp
    IVAsioPeer.hpp
    VAsioDatatypes.hpp
    VAsioMsgKind.hpp
    VAsioPeerInfo.hpp
    VAsioReceiver.hpp
    VAsioRegistry.hpp
    VAsioRegistry.cpp
    VAsioTcpPeer.hpp
    VAsioTcpPeer.cpp
    VAsioTransmitter.hpp

    SerializedMessageTraits.hpp
    SerializedMessage.hpp
    SerializedMessage.cpp
)

target_link_libraries(S_SilKit_Core_VAsio
    PUBLIC I_SilKit_Core_VAsio

    PUBLIC
      I_SilKit_Util
      SilKitInterface
      I_SilKit_Core_Internal
      S_SilKit_Services_Orchestration
      S_SilKit_Services_Logging
      S_SilKit_Core_Service
      S_SilKit_Services_PubSub
      S_SilKit_Services_Rpc
      S_SilKit_Services_Can
      S_SilKit_Util_Uri
      S_SilKit_Services_Ethernet
      S_SilKit_Services_Lin
      S_SilKit_Services_Flexray
      Threads::Threads
      S_SilKit_Util_Filesystem

    INTERFACE I_SilKit_Config
)

target_compile_definitions(S_SilKit_Core_VAsio PRIVATE ASIO_STANDALONE)

target_include_directories(S_SilKit_Core_VAsio
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)


if (MSVC)
    target_compile_options(S_SilKit_Core_VAsio PRIVATE "/bigobj")
    target_compile_definitions(I_SilKit_Core_VAsio INTERFACE _WIN32_WINNT=0x0601)
endif()
if(MINGW)
    target_compile_definitions(I_SilKit_Core_VAsio INTERFACE _WIN32_WINNT=0x0601)
    target_link_libraries(S_SilKit_Core_VAsio PUBLIC -lwsock32 -lws2_32) #windows socket/ wsa
endif()

add_silkit_test(Test_MwVAsioConnection SOURCES Test_VAsioConnection.cpp LIBS S_SilKit_Core_VAsio I_SilKit_Core_Mock_Participant)

add_silkit_test(Test_MwVAsio_Serdes  SOURCES Test_VAsioSerdes.cpp LIBS S_SilKit_Core_VAsio)
add_silkit_test(Test_MwVAsio_SerializedMessage  SOURCES Test_SerializedMessage.cpp LIBS S_SilKit_Core_VAsio)
add_silkit_test(Test_MwVAsio_Uri  SOURCES Test_Uri.cpp LIBS S_SilKit_Core_VAsio)

# Testing interoperability between different protocol versions requires testing on a higher level:
# We instantiate a complete Participant<VAsioConnection> with a specific version
# and do integration tests here
add_silkit_test(ITest_ParticipantVersion SOURCES Test_ParticipantVersion.cpp LIBS S_SilKit_Core_Participant S_ITests_STH)

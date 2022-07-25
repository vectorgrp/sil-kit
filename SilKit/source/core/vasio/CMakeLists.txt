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

add_library(SilKitMwVAsio STATIC
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

    Uri.hpp
    Uri.cpp

    SerializedMessageTraits.hpp
    SerializedMessage.hpp
    SerializedMessage.cpp
)

target_link_libraries(SilKitMwVAsio
    PUBLIC
      SilKitUtil
      SilKitInterface
      SilKitInternal
      SilKitMwSync
      SilKitMwLogging
      SilKitMwService
      SilKitSimDataMessage
      SilKitSimRpc
      SilKitSimCan
      SilKitSimEthernet
      SilKitSimLin
      SilKitSimFlexray
      Threads::Threads
      SilKitFilesystem
      asio
    INTERFACE
      SilKitConfiguration
)

target_compile_definitions(SilKitMwVAsio PRIVATE ASIO_STANDALONE)

target_include_directories(SilKitMwVAsio
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)


if (MSVC)
    target_compile_options(SilKitMwVAsio PRIVATE "/bigobj")
    target_compile_definitions(SilKitMwVAsio PUBLIC _WIN32_WINNT=0x0601)
endif()
if(MINGW)
    target_compile_definitions(SilKitMwVAsio PUBLIC _WIN32_WINNT=0x0601)
    target_link_libraries(SilKitMwVAsio PUBLIC -lwsock32 -lws2_32) #windows socket/ wsa
endif()

add_silkit_test(TestMwVAsioConnection SOURCES VAsioConnectionTest.cpp LIBS SilKitMwVAsio SilKitMwMockParticipant)

add_silkit_test(TestMwVAsio_Serdes  SOURCES VAsioSerdesTest.cpp LIBS SilKitMwVAsio)
add_silkit_test(TestMwVAsio_SerializedMessage  SOURCES SerializedMessageTest.cpp LIBS SilKitMwVAsio)
add_silkit_test(TestMwVAsio_Uri  SOURCES UriTest.cpp LIBS SilKitMwVAsio)

# Testing interoperability between different protocol versions requires testing on a higher level:
# We instantiate a complete Participant<VAsioConnection> with a specific version
# and do integration tests here
add_silkit_test(ITest_ParticipantVersion  SOURCES ParticipantVersionTest.cpp LIBS SilKitMwParticipant SilKitSimTestHarness)

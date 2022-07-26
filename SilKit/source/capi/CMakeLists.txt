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

add_silkit_test(TestCapi_Interfaces SOURCES CapiInterfacesTest.cpp LIBS SilKit)
add_silkit_test(TestCapiCan SOURCES CapiCanTest.cpp LIBS SilKitSimCan SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiEthernet SOURCES CapiEthernetTest.cpp LIBS SilKitSimEthernet SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiFlexray SOURCES CapiFlexrayTest.cpp LIBS SilKitSimFlexray SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiData SOURCES CapiDataTest.cpp LIBS SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiRpc SOURCES CapiRpcTest.cpp LIBS SilKitSimRpc SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiLogger SOURCES CapiLoggerTest.cpp LIBS SilKitMwLogging SilKit)

add_silkit_test(TestCapiSilKit SOURCES CapiSilKitTest.cpp LIBS SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiParticipantStateHandling SOURCES CapiParticipantStateHandlingTest.cpp LIBS SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiTimeSync SOURCES CapiTimeSyncTest.cpp LIBS SilKitMwMockParticipant SilKit)
add_silkit_test(TestCapiLin SOURCES CapiLinTest.cpp LIBS SilKitSimLin SilKitMwMockParticipant SilKit)
add_silkit_test(Test_CapiSymbols SOURCES Test_CapiSymbols.cpp LIBS SilKit)


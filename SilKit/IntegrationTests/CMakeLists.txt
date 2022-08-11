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

# local headers as interface lib
add_subdirectory(SimTestHarness)

add_library(ITest_Utils INTERFACE)
target_include_directories(ITest_Utils
    INTERFACE ${CMAKE_CURRENT_LIST_DIR})

set(itest_extra_libs SilKitMwVAsio)

add_silkit_test(FTest_CanWithoutSync
    SOURCES
      FTest_CanWithoutSync.cpp
    LIBS
      SilKitSimCan
      ${itest_extra_libs}
      SilKit
      SilKitConfigurationTestUtils
)

add_silkit_test(FTest_EthWithoutSync
    SOURCES
      FTest_EthWithoutSync.cpp
    LIBS
      SilKitSimEthernet
      ${itest_extra_libs}
      SilKit
      SilKitConfigurationTestUtils
)

add_silkit_test(ITest_ThreeCanController
    SOURCES
      ITest_ThreeCanController.cpp
    LIBS
        SilKitMwParticipant
        SilKitSimTestHarness
        ${itest_extra_libs}
)

add_silkit_test(FTest_CanControllerThreadSafety
    SOURCES
      FTest_CanControllerThreadSafety.cpp
    LIBS
        SilKit
        SilKitConfigurationTestUtils
)

add_silkit_test(ITest_ThreeEthController
    SOURCES
      ITest_ThreeEthController.cpp
    LIBS
      SilKitSimTestHarness
      SilKitMwParticipant
      SilKitSimEthernet
      ${itest_extra_libs}
)

add_silkit_test(ITest_CatchExceptionsInCallbacks
    SOURCES
      ITest_CatchExceptionsInCallbacks.cpp
    LIBS
      SilKitMwParticipant
      SilKitConfigurationTestUtils
      ${itest_extra_libs}
)

add_silkit_test(ITest_HopOnHopOff
    SOURCES
      ITest_HopOnHopOff.cpp
    LIBS
        SilKit
        SilKitConfigurationTestUtils
        SilKitInternal
)

add_silkit_test(ITest_CommunicationReady
    SOURCES
      ITest_CommunicationReady.cpp
    LIBS
        SilKit
        SilKitConfigurationTestUtils
        SilKitInternal
)

add_silkit_test(ITest_DataPubSub
    SOURCES
      ITest_DataPubSub.cpp
    LIBS
        SilKit
        SilKitInternal
        SilKitConfigurationTestUtils
)

add_silkit_test(ITest_TargetedMessaging
    SOURCES
      ITest_TargetedMessaging.cpp
    LIBS
      SilKitTracing
      SilKitSimCan
      SilKitSimTestHarness
      SilKitInternal
)

add_silkit_test(ITest_Rpc
    SOURCES
      ITest_Rpc.cpp
    LIBS
      SilKit
      SilKitInternal
      SilKitConfigurationTestUtils
)

add_silkit_test(ITest_StateMachineVAsio
    SOURCES
      ITest_StateMachineVAsio.cpp
    LIBS
      SilKitMwParticipant
      SilKitMwVAsio
      SilKitConfigurationTestUtils
)

add_silkit_test(ITest_DeterministicSimVAsio
    SOURCES
      ITest_DeterministicSimVAsio.cpp
    LIBS
      SilKitMwParticipant
      SilKitMwVAsio
      SilKitConfigurationTestUtils
)

add_silkit_test(ITest_DifferentPeriods
    SOURCES
      ITest_DifferentPeriods.cpp
    LIBS
      SilKitMwParticipant
      SilKitMwVAsio
      SilKitConfigurationTestUtils
)

add_silkit_test(ITest_SingleParticipant
    SOURCES
      ITest_SingleParticipant.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      SilKitMwVAsio
)

add_silkit_test(ITest_ServiceDiscovery
    SOURCES
      ITest_ServiceDiscovery.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      ${itest_extra_libs}
)

add_silkit_test(FTest_ServiceDiscoveryPerf
    SOURCES
      FTest_ServiceDiscoveryPerf.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      ${itest_extra_libs}
)

add_silkit_test(ITest_SystemMonitor
    SOURCES
      ITest_SystemMonitor.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      ${itest_extra_libs}
)

add_silkit_test(FTest_PubSubPerf
    SOURCES
      FTest_PubSubPerf.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      ${itest_extra_libs}
)

add_silkit_test(ITest_AsyncSimTask
    SOURCES
      ITest_AsyncSimTask.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      ${itest_extra_libs}
)

add_silkit_test(ITest_Lin
    SOURCES
      ITest_Lin.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      ${itest_extra_libs}
)

# Windows specific tests
if(MSVC)
    add_silkit_test(ITest_CompatibilityWithWindowsHeaders
        SOURCES
        ITest_CompatibilityWithWindowsHeaders.cpp
        LIBS
        SilKitMwParticipant
        )
endif()


# ITest_s based on demos

add_silkit_test(ITest_LinDemo SOURCES ITest_LinDemo.cpp LIBS SilKitSimTestHarness SilKitMwParticipant ${itest_extra_libs})
add_silkit_test(ITest_CanDemo SOURCES ITest_CanDemo.cpp LIBS SilKitSimTestHarness SilKitMwParticipant ${itest_extra_libs})
add_silkit_test(ITest_EthernetDemo SOURCES ITest_EthernetDemo.cpp LIBS SilKitSimTestHarness SilKitMwParticipant ${itest_extra_libs})

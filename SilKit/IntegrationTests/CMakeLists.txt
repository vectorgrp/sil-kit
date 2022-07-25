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

add_library(ITestUtils INTERFACE)
target_include_directories(ITestUtils
    INTERFACE ${CMAKE_CURRENT_LIST_DIR})

set(itest_extra_libs SilKitMwVAsio)

add_silkit_test(FTestCanWithoutSync
    SOURCES
      CanWithoutSyncFTest.cpp
    LIBS
      SilKitSimCan
      ${itest_extra_libs}
      SilKit
      SilKitConfigurationTestUtils
)

add_silkit_test(FTestEthWithoutSync
    SOURCES
      EthWithoutSyncFTest.cpp
    LIBS
      SilKitSimEthernet
      ${itest_extra_libs}
      SilKit
      SilKitConfigurationTestUtils
)

add_silkit_test(ITestThreeCanController
    SOURCES
      ThreeCanControllerITest.cpp
    LIBS
        SilKitMwParticipant
        SilKitSimTestHarness
        ${itest_extra_libs}
)

add_silkit_test(FTestCanControllerThreadSafety
    SOURCES
      CanControllerThreadSafetyITest.cpp
    LIBS
        SilKit
        SilKitConfigurationTestUtils
)

add_silkit_test(ITestThreeEthController
    SOURCES
      ThreeEthControllerITest.cpp
    LIBS
      SilKitSimTestHarness
      SilKitMwParticipant
      SilKitSimEthernet
      ${itest_extra_libs}
)

add_silkit_test(ITestCatchExceptionsInCallbacks
    SOURCES
      CatchExceptionsInCallbacksITest.cpp
    LIBS
      SilKitMwParticipant
      SilKitConfigurationTestUtils
      ${itest_extra_libs}
)

add_silkit_test(ITestHopOnHopOff
    SOURCES
      HopOnHopOffITest.cpp
    LIBS
        SilKit
        SilKitConfigurationTestUtils
)

add_silkit_test(ITestDataPubSub
    SOURCES
      DataPubSubITest.cpp
    LIBS
        SilKit
        SilKitInternal
        SilKitConfigurationTestUtils
)

add_silkit_test(ITestTargetedMessaging
    SOURCES
      TargetedMessagingITest.cpp
    LIBS
      SilKitTracing
      SilKitSimCan
      SilKitSimTestHarness
      SilKitInternal
)

add_silkit_test(ITestRpc
    SOURCES
      RpcITest.cpp
    LIBS
      SilKit
      SilKitInternal
      SilKitConfigurationTestUtils
)

add_silkit_test(ITestStateMachineVAsio
    SOURCES
      StateMachineVAsioITest.cpp
    LIBS
      SilKitMwParticipant
      SilKitMwVAsio
      SilKitConfigurationTestUtils
)

add_silkit_test(ITestDeterministicSimVAsio
    SOURCES
      DeterministicSimVAsioITest.cpp
    LIBS
      SilKitMwParticipant
      SilKitMwVAsio
      SilKitConfigurationTestUtils
)

add_silkit_test(ITestDifferentPeriods
    SOURCES
      DifferentPeriodsITest.cpp
    LIBS
      SilKitMwParticipant
      SilKitMwVAsio
      SilKitConfigurationTestUtils
)

add_silkit_test(ITestSingleParticipant
    SOURCES
      SingleParticipantITest.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      SilKitMwVAsio
)

add_silkit_test(ITestServiceDiscovery
    SOURCES
      ServiceDiscoveryITest.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      ${itest_extra_libs}
)

add_silkit_test(FTestServiceDiscoveryPerf
    SOURCES
      ServiceDiscoveryPerfFTest.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      ${itest_extra_libs}
)

add_silkit_test(ITestSystemMonitor
    SOURCES
      SystemMonitorITest.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      ${itest_extra_libs}
)

add_silkit_test(FTestPubSubPerf
    SOURCES
      PubSubPerfFTest.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      ${itest_extra_libs}
)

add_silkit_test(ITestAsyncSimTask
    SOURCES
      AsyncSimTaskITest.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      ${itest_extra_libs}
)

add_silkit_test(ITestLin
    SOURCES
      LinITest.cpp
    LIBS
      SilKitMwParticipant
      SilKitSimTestHarness
      ${itest_extra_libs}
)

# Windows specific tests
if(MSVC)
    add_silkit_test(ITestCompatibilityWithWindowsHeaders
        SOURCES
        CompatibilityWithWindowsHeadersITest.cpp
        LIBS
        SilKitMwParticipant
        )
endif()


# ITests based on demos

add_silkit_test(ITestLinDemo SOURCES LinDemoITest.cpp LIBS SilKitSimTestHarness SilKitMwParticipant ${itest_extra_libs})
add_silkit_test(ITestCanDemo SOURCES CanDemoITest.cpp LIBS SilKitSimTestHarness SilKitMwParticipant ${itest_extra_libs})
add_silkit_test(ITestEthernetDemo SOURCES EthernetDemoITest.cpp LIBS SilKitSimTestHarness SilKitMwParticipant ${itest_extra_libs})

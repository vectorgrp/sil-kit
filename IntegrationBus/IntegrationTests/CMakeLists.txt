# Copyright (c) Vector Informatik GmbH. All rights reserved.
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

add_silkit_test(ITestSilKitRegistryReusage
    SOURCES
      SilKitRegistryReusage.cpp
    LIBS
      SilKitMwParticipant
      SilKitMwVAsio
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

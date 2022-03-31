# Copyright (c) Vector Informatik GmbH. All rights reserved.
# local headers as interface lib
add_subdirectory(SimTestHarness)

add_library(ITestUtils INTERFACE)
target_include_directories(ITestUtils
    INTERFACE ${CMAKE_CURRENT_LIST_DIR})

set(itest_extra_libs)
if(IB_MW_ENABLE_VASIO)
    set(itest_extra_libs IbMwVAsio)
endif()

add_vib_test(FTestCanWithoutSync
    SOURCES
      CanWithoutSyncFTest.cpp
    LIBS
      IbSimCan
      ${itest_extra_libs}
      IntegrationBus
      IbCfgMockParticipantConfiguration
)

add_vib_test(FTestEthWithoutSync
    SOURCES
      EthWithoutSyncFTest.cpp
    LIBS
      IbSimEthernet
      ${itest_extra_libs}
      IntegrationBus
      IbCfgMockParticipantConfiguration
)

add_vib_test(ITestThreeCanController
    SOURCES
      ThreeCanControllerITest.cpp
    LIBS
        IbMwComAdapter
        IbSimTestHarness
        ${itest_extra_libs}
)

add_vib_test(ITestThreeEthController
    SOURCES
      ThreeEthControllerITest.cpp
    LIBS
      IbSimTestHarness
      IbMwComAdapter
      ${itest_extra_libs}
)

add_vib_test(ITestCatchExceptionsInCallbacks
    SOURCES
      CatchExceptionsInCallbacksITest.cpp
    LIBS
      IbMwComAdapter
      IbCfgMockParticipantConfiguration
      ${itest_extra_libs}
)

add_vib_test(ITestHopOnHopOff
    SOURCES
      HopOnHopOffITest.cpp
    LIBS
        IntegrationBus
        IbCfgMockParticipantConfiguration
)
 
add_vib_test(ITestDataPubSub
    SOURCES
      DataPubSubITest.cpp
    LIBS
        IntegrationBus
        IbCfgMockParticipantConfiguration
)

add_vib_test(ITestTargetedMessaging
    SOURCES
      TargetedMessagingITest.cpp
    LIBS
      IbTracing
      IbSimCan 
      IbSimTestHarness
      IbInternal
)

add_vib_test(ITestRpc
    SOURCES
      RpcITest.cpp
    LIBS
      IntegrationBus
      IbCfgMockParticipantConfiguration
)

if(IB_MW_ENABLE_VASIO)
    add_vib_test(ITestStateMachineVAsio
        SOURCES
          StateMachineVAsioITest.cpp
        LIBS
          IbMwComAdapter
          IbMwVAsio
          IbCfgMockParticipantConfiguration
    )

    add_vib_test(ITestDeterministicSimVAsio
        SOURCES
          DeterministicSimVAsioITest.cpp
        LIBS
          IbMwComAdapter
          IbMwVAsio
          IbCfgMockParticipantConfiguration
    )

    add_vib_test(ITestDifferentPeriods
        SOURCES
          DifferentPeriodsITest.cpp
        LIBS
          IbMwComAdapter
          IbMwVAsio
          IbCfgMockParticipantConfiguration
    )

    add_vib_test(ITestIbRegistryReusage
        SOURCES
          IbRegistryReusage.cpp
        LIBS
          IbMwComAdapter
          IbMwVAsio
    )

    add_vib_test(ITestSingleParticipant
        SOURCES
          SingleParticipantITest.cpp
        LIBS
          IbMwComAdapter
          IbSimTestHarness
          IbMwVAsio
    )
endif()

add_vib_test(ITestServiceDiscovery
    SOURCES
      ServiceDiscoveryITest.cpp
    LIBS
      IbMwComAdapter
      IbSimTestHarness
      ${itest_extra_libs}
)

add_vib_test(FTestServiceDiscoveryPerf
    SOURCES
      ServiceDiscoveryPerfFTest.cpp
    LIBS
      IbMwComAdapter
      IbSimTestHarness
      ${itest_extra_libs}
)
add_vib_test(ITestAsyncSimTask
    SOURCES
      AsyncSimTaskITest.cpp
    LIBS
      IbMwComAdapter
      IbSimTestHarness
      ${itest_extra_libs}
)

add_vib_test(ITestLin
    SOURCES
      LinITest.cpp
    LIBS
      IbMwComAdapter
      IbSimTestHarness
      ${itest_extra_libs}
)

# Windows specific tests
if(WIN32)
    add_vib_test(ITestCompatibilityWithWindowsHeaders
        SOURCES
        CompatibilityWithWindowsHeadersITest.cpp
        LIBS
        IbMwComAdapter
        )
endif()

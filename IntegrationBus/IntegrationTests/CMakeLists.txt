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
)

add_vib_test(FTestEthWithoutSync
    SOURCES
      EthWithoutSyncFTest.cpp
    LIBS
      IbSimEthernet
      ${itest_extra_libs}
      IntegrationBus
)

add_vib_test(ITestGenericMessages
    SOURCES
      GenericMessagesITest.cpp
    LIBS
      IbMwComAdapter
      ${itest_extra_libs}
    CONFIGS
      GenericMessagesITest_IbConfig.json
)

add_vib_test(ITestLargeMessages
    SOURCES
        LargeMessagesITest.cpp
    LIBS
        IbMwComAdapter
        ${itest_extra_libs}
    CONFIGS
        LargeMessagesITest_IbConfig.json
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

add_vib_test(ITestIoMessages
    SOURCES
      IoMessagesITest.cpp
    LIBS
        IbSimTestHarness
)

add_vib_test(ITestCatchExceptionsInCallbacks
    SOURCES
      CatchExceptionsInCallbacksITest.cpp
    LIBS
      IbMwComAdapter
      ${itest_extra_libs}
)

add_vib_test(ITestHopOnHopOff
    SOURCES
      HopOnHopOffITest.cpp
    LIBS
        IntegrationBus
)
 
add_vib_test(FTestDataPubSub
    SOURCES
      DataPubSubITest.cpp
    LIBS
        IntegrationBus
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

if(IB_MW_ENABLE_VASIO)
    add_vib_test(ITestStateMachineVAsio
        SOURCES
          StateMachineVAsioITest.cpp
        LIBS
          IbMwComAdapter
          IbMwVAsio
    )

    add_vib_test(ITestDeterministicSimVAsio
        SOURCES
          DeterministicSimVAsioITest.cpp
        LIBS
          IbMwComAdapter
          IbMwVAsio
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
          IbSimTestHarness
          IbMwVAsio
    )
endif()

add_vib_test(FTestServiceDiscovery
    SOURCES
      ServiceDiscoveryFTest.cpp
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

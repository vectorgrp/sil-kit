add_vib_test(ITestGenericMessages
    SOURCES
      GenericMessagesITest.cpp
    LIBS
      IbMwComAdapter
      IbMwVAsio
    CONFIGS
      GenericMessagesITest_IbConfig.json
      LargeMessagesITest_IbConfig.json
      LargeMessagesITest_FastRTPS.xml
)

add_vib_test(ITestThreeCanController
    SOURCES
      ThreeCanControllerITest.cpp
    LIBS
      IbMwComAdapter
      IbMwVAsio
)

add_vib_test(ITestThreeEthController
    SOURCES
      ThreeEthControllerITest.cpp
    LIBS
      IbMwComAdapter
      IbMwVAsio
)

add_vib_test(ITestSocketBufferSize
    SOURCES
      SocketBufferSizeITest.cpp
    LIBS
      IbMwComAdapter
    CONFIGS
      SocketBufferSizeITest_IbConfig.json
      SocketBufferSizeITest_FastRTPS.xml
)

add_vib_test(ITestWaitForAllAcked
    SOURCES
      WaitForAllAckedITest.cpp
    LIBS
      IbMwComAdapter
)

add_vib_test(ITestIoMessages
    SOURCES
      IoMessagesITest.cpp
    LIBS
      IbMwComAdapter
      IbMwVAsio
)

add_vib_test(ITestCatchExceptionsInCallbacks
    SOURCES
      CatchExceptionsInCallbacksITest.cpp
    LIBS
      IbMwComAdapter
      IbMwVAsio
)

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


add_vib_test(ITestTimeProvider
    SOURCES
      TimeProviderITest.cpp
    LIBS
      IbMwComAdapter
      IbMwVAsio
)

add_vib_test(ITestTracing
    SOURCES
      TracingITest.cpp
    LIBS
      IbMwComAdapter
      IbMwVAsio
      IbTracing
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

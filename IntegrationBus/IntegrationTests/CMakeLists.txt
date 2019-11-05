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

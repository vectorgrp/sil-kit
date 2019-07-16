add_vib_test(ITestGenericMessages SOURCES GenericMessagesITest.cpp LIBS
    IbMwComAdapter
    IbMwRegistry
    IbConfig
    IbUtil
    IbMwSync
    IbSimCan
    IbSimEthernet
    IbSimFlexRay
    IbSimLin
    IbSimIO
    IbSimGenericMessage CONFIGS
    GenericMessagesITest_IbConfig.json
    LargeMessagesITest_IbConfig.json
    LargeMessagesITest_FastRTPS.xml
)

add_vib_test(ITestSocketBufferSize SOURCES SocketBufferSizeITest.cpp LIBS
    IbMwComAdapter
    IbConfig
    IbUtil
    IbMwSync
    IbSimCan
    IbSimEthernet
    IbSimFlexRay
    IbSimLin
    IbSimIO
    IbSimGenericMessage
    CONFIGS SocketBufferSizeITest_IbConfig.json SocketBufferSizeITest_FastRTPS.xml
)

add_vib_test(ITestWaitForAllAcked SOURCES WaitForAllAckedITest.cpp LIBS
    IbMwComAdapter
    IbConfig
    IbUtil
    IbMwSync
    IbSimCan
    IbSimEthernet
    IbSimFlexRay
    IbSimLin
    IbSimIO
    IbSimGenericMessage
)

add_vib_test(ITestIoMessages SOURCES IoMessagesITest.cpp LIBS
    IbMwComAdapter
	IbMwRegistry
    IbConfig
    IbUtil
    IbMwSync
    IbSimCan
    IbSimEthernet
    IbSimFlexRay
    IbSimLin
    IbSimIO
    IbSimGenericMessage
)

add_vib_test(ITestCatchExceptionsInCallbacks SOURCES CatchExceptionsInCallbacksITest.cpp LIBS
    IbMwComAdapter
	IbMwRegistry
    IbConfig
    IbUtil
    IbMwSync
    IbSimCan
    IbSimEthernet
    IbSimFlexRay
    IbSimLin
    IbSimIO
    IbSimGenericMessage
)

add_vib_test(ITestStateMachineVAsio SOURCES StateMachineVAsioITest.cpp LIBS
    IbMwComAdapter
    IbMwRegistry
    IbConfig
    IbUtil
    IbMwSync
    IbSimGenericMessage
)

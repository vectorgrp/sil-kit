# local headers as interface lib
add_subdirectory(SimTestHarness)

add_library(ITestUtils INTERFACE)
target_include_directories(ITestUtils
    INTERFACE ${CMAKE_CURRENT_LIST_DIR})

set(itest_extra_libs)
if(IB_MW_ENABLE_VASIO)
    set(itest_extra_libs IbMwVAsio)
endif()

add_vib_test(ITestGenericMessages
    SOURCES
      GenericMessagesITest.cpp
    LIBS
      IbMwComAdapter
      ${itest_extra_libs}
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
        IbSimTestHarness
        ${itest_extra_libs}
)

# requires IntegrationBus.dll and vib-regstry to be in PATH or CWD
set_tests_properties(ITestThreeCanController PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_vib_test(ITestThreeEthController
    SOURCES
      ThreeEthControllerITest.cpp
    LIBS
      IbSimTestHarness
      IbMwComAdapter
      ${itest_extra_libs}
)

# requires IntegrationBus.dll and vib-regstry to be in PATH or CWD
set_tests_properties(ITestThreeEthController PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
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
        IbSimTestHarness
)

# requires IntegrationBus.dll and vib-regstry to be in PATH or CWD
set_tests_properties(ITestIoMessages PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)


add_vib_test(ITestCatchExceptionsInCallbacks
    SOURCES
      CatchExceptionsInCallbacksITest.cpp
    LIBS
      IbMwComAdapter
      ${itest_extra_libs}
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
endif()

# Windows specific tests
if(WIN32)
    add_vib_test(ITestCompatibilityWithWindowsHeaders
        SOURCES
        CompatibilityWithWindowsHeadersITest.cpp
        LIBS
        IbMwComAdapter
        )
endif()

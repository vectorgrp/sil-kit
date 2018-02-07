# We expect to be called as follows:
# cmake -DPATH=PATH -P GTestRunner.cmake TestExecutable Arg1 Arg2 Arg3

set(ENV{PATH} "${PATH}")
set(TEST_CMD ${CMAKE_ARGV4})

execute_process(
    COMMAND ${TEST_CMD} --gtest_output=xml:${TEST_CMD}_gtestresults.xml ${CMAKE_ARGV5} ${CMAKE_ARGV6} ${CMAKE_ARGV7}
    RESULT_VARIABLE CTEST_RETVAL
)
if(NOT CTEST_RETVAL EQUAL 0)
    message(FATAL_ERROR "IntegrationTest ${TEXT_CMD} FAILED")
endif()

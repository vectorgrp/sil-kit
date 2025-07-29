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

################################################################################
# Helper Functions
################################################################################

set(_SilKitTest_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "" FORCE)

function(add_silkit_test_executable SILKIT_TEST_EXECUTABLE_NAME)
    if(NOT ${SILKIT_BUILD_TESTS})
        return()
    endif()

    # If we can bump our required CMake version to 3.17 we can use
    # CMAKE_CURRENT_FUNCTION_LIST_DIR
    # to get the directory this file (SilKitTest.cmake) resides in.

    add_executable(
        "${SILKIT_TEST_EXECUTABLE_NAME}"
        "${_SilKitTest_BASE_DIR}/SilKitTest/SilKitTestMain.cpp"
    )

    target_link_libraries("${SILKIT_TEST_EXECUTABLE_NAME}"
        PRIVATE SilKitInterface
        PRIVATE gtest
        PRIVATE gmock
    )

    target_compile_definitions("${SILKIT_TEST_EXECUTABLE_NAME}"
        PRIVATE UNIT_TEST
    )

    set_property(TARGET "${SILKIT_TEST_EXECUTABLE_NAME}" PROPERTY FOLDER "Tests")

    set(dummyConfig "${CMAKE_CURRENT_BINARY_DIR}/${SILKIT_TEST_EXECUTABLE_NAME}.dummyconfig")
    file(TOUCH "${dummyConfig}")

    set_target_properties("${SILKIT_TEST_EXECUTABLE_NAME}" PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
        SILKIT_TEST_CONFIGURATION_FILES "${dummyConfig}"
    )

    add_custom_command(
        TARGET ${SILKIT_TEST_EXECUTABLE_NAME} POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy
                $<TARGET_PROPERTY:${SILKIT_TEST_EXECUTABLE_NAME},SILKIT_TEST_CONFIGURATION_FILES>
                $<TARGET_FILE_DIR:${SILKIT_TEST_EXECUTABLE_NAME}>/
        COMMAND_EXPAND_LISTS
    )

    if (MSVC)
        target_compile_options("${SILKIT_TEST_EXECUTABLE_NAME}" PRIVATE "/bigobj")
    endif(MSVC)
endfunction()

function(add_silkit_test_to_executable SILKIT_TEST_EXECUTABLE_NAME)
    if(NOT ${SILKIT_BUILD_TESTS})
        return()
    endif()

    set(mva SOURCES LIBS CONFIGS TESTSUITE_NAME)

    cmake_parse_arguments(arg
        ""
        ""
        "${mva}"
        ${ARGN}
    )

    target_sources("${SILKIT_TEST_EXECUTABLE_NAME}" PRIVATE ${arg_SOURCES})

    target_link_libraries("${SILKIT_TEST_EXECUTABLE_NAME}" PRIVATE ${arg_LIBS})

    foreach(config ${arg_CONFIGS})
        get_filename_component(configPath "${config}" ABSOLUTE)
        set_property(TARGET "${SILKIT_TEST_EXECUTABLE_NAME}" APPEND PROPERTY SILKIT_TEST_CONFIGURATION_FILES "${configPath}")
    endforeach()

    # initialize the list of test suite names with the multi-value argument
    set(testSuiteNames "${arg_TESTSUITE_NAME}")

    # add each matching TU name as a test suite name
    foreach(translationUnit ${arg_SOURCES})
        if (NOT ${translationUnit} MATCHES .*Test_.*\.cpp$)
            continue()
        endif ()

        get_filename_component(tuName "${translationUnit}" NAME_WE)
        list(APPEND testSuiteNames "${tuName}")
    endforeach()

    # strip each test suite name and remove duplicates
    list(TRANSFORM testSuiteNames STRIP)
    list(REMOVE_DUPLICATES testSuiteNames)

    # check that the test has at least a single test suite name
    list(LENGTH testSuiteNames numberOfTestSuiteNames)
    if (numberOfTestSuiteNames EQUAL 0)
        message(FATAL_ERROR "SIL Kit: No test suites were added to ${SILKIT_TEST_EXECUTABLE_NAME}")
    endif ()

    # use the test suite names to add tests (CTest)
    foreach (testSuite ${testSuiteNames})
        message(VERBOSE "-- SIL Kit: CTest ${testSuite} (${SILKIT_TEST_EXECUTABLE_NAME})")

        add_test(
            NAME "${testSuite}"
            COMMAND
                "${SILKIT_TEST_EXECUTABLE_NAME}"
                "--gtest_output=xml:${testSuite}_gtestresults.xml"
                "--gtest_filter=${testSuite}.*"
            WORKING_DIRECTORY $<TARGET_FILE_DIR:${SILKIT_TEST_EXECUTABLE_NAME}>
        )
    endforeach ()
endfunction()

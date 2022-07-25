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

function(add_silkit_test)
    if(NOT ${SILKIT_BUILD_TESTS})
        return()
    endif()

    set(multiValueArgs SOURCES LIBS CONFIGS)

    cmake_parse_arguments(PARSED_ARGS
        ""
        ""
        "${multiValueArgs}"
        ${ARGN}
    )

    if(NOT PARSED_ARGS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "add_silkit_test function failed because no executable name was specified (UNPARSED_ARGUMENTS were empty).")
    endif()

    list(GET PARSED_ARGS_UNPARSED_ARGUMENTS 0 executableName)

    if(NOT PARSED_ARGS_SOURCES)
        message(FATAL_ERROR "add_silkit_test function for ${executableName} has an empty source list.")
    endif()

    add_executable(${executableName}
        ${PARSED_ARGS_SOURCES}
    )

    set_property(TARGET ${executableName} PROPERTY FOLDER "Tests")

    target_link_libraries(${executableName}
        PRIVATE SilKitInterface
        gtest
        gmock_main
        ${PARSED_ARGS_LIBS}
    )
    target_compile_definitions(${executableName}
        PRIVATE
        UNIT_TEST
    )
    set_target_properties(${executableName} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    )

    foreach(config ${PARSED_ARGS_CONFIGS})
        get_filename_component(configFileName ${config} NAME)
        add_custom_command(
            TARGET ${executableName} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/${config} $<TARGET_FILE_DIR:${executableName}>/${configFileName}
        )
    endforeach()

    if (MSVC)
        target_compile_options(${executableName} PRIVATE "/bigobj")
    endif(MSVC)

    add_test(NAME ${executableName}
             COMMAND ${executableName} --gtest_output=xml:${executableName}_gtestresults.xml
             WORKING_DIRECTORY $<TARGET_FILE_DIR:${executableName}>
    )
    #ensure test execution has the MinGW libraries in PATH
    if(MINGW)
        get_filename_component(compilerDir ${CMAKE_CXX_COMPILER} DIRECTORY)
        set_tests_properties(${executableName} PROPERTIES ENVIRONMENT "PATH=${compilerDir};")
    endif()
endfunction(add_silkit_test)

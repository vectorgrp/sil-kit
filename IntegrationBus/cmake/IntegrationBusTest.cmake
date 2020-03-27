################################################################################
# Helper Functions
################################################################################

function(add_vib_test)
    if(NOT ${IB_BUILD_TESTS})
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
        message(FATAL_ERROR "add_ib_test function failed because no executable name was specified (UNPARSED_ARGUMENTS were empty).")
    endif()

    list(GET PARSED_ARGS_UNPARSED_ARGUMENTS 0 executableName)

    if(NOT PARSED_ARGS_SOURCES)
        message(FATAL_ERROR "add_ib_test function for ${executableName} has an empty source list.")
    endif()

    add_executable(${executableName}
        ${PARSED_ARGS_SOURCES}
    )

    set_property(TARGET ${executableName} PROPERTY FOLDER "Tests")

    target_link_libraries(${executableName}
        PRIVATE IbInterface
        gtest
        gmock_main
        ${PARSED_ARGS_LIBS}
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
    else()
        target_compile_options(${executableName} PUBLIC "-Wno-inconsistent-missing-override")
    endif(MSVC)

    add_test(NAME ${executableName}
             COMMAND ${executableName} --gtest_output=xml:${executableName}_gtestresults.xml
             WORKING_DIRECTORY $<TARGET_FILE_DIR:${executableName}>
    )
endfunction(add_vib_test)


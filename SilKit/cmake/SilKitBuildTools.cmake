# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

include(CMakeFindBinUtils)

macro(silkit_split_debugsymbols targetName)
    set(_targetFile "$<TARGET_FILE:${targetName}>")
    set(_debugFile "${SILKIT_SYMBOLS_DIR}/${targetName}${CMAKE_DEBUG_POSTFIX}.debug")
    add_custom_command(
        TARGET ${targetName}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --only-keep-debug "${_targetFile}" "${_debugFile}"
        COMMAND ${CMAKE_STRIP} "${_targetFile}"
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink="${_debugFile}" "${_targetFile}"
        COMMENT "SIL Kit: SILKIT_PACKAGE_SYMBOLS: splitting ELF debug symbols"
    )
endmacro()

macro(silkit_package_debugsymbols targetName)
    if(MSVC)
        message(STATUS "Creating symbol package ${SILKIT_SYMBOLS_DIR_NAME}")
        add_custom_command(
            TARGET "${targetName}"
            POST_BUILD
            WORKING_DIRECTORY "${SILKIT_SYMBOLS_DIR_BASE}"
            COMMAND "${CMAKE_COMMAND}" -E tar cf "${SILKIT_SYMBOLS_DIR_NAME}.zip" --format=zip -- "${SILKIT_SYMBOLS_DIR_NAME}"
            COMMENT "SIL Kit: SILKIT_PACKAGE_SYMBOLS: creating PDB zip"
        )
    endif()
    if(APPLE)
        return()
    endif()
    if(UNIX AND CMAKE_BUILD_TYPE MATCHES "Debug")
        get_target_property(targetType ${targetName} TYPE)
        if(targetType STREQUAL STATIC_LIBRARY)
            message(STATUS "SIL Kit: splitting debug symbols on static libraries is not supported")
            return()
        endif()

        silkit_split_debugsymbols("${targetName}")

        file(MAKE_DIRECTORY "${SILKIT_SYMBOLS_DIR}")

        message(STATUS "Creating symbol package ${SILKIT_SYMBOLS_DIR_NAME}")
        add_custom_command(
            TARGET "${targetName}"
            POST_BUILD
            WORKING_DIRECTORY "${SILKIT_SYMBOLS_DIR_BASE}"
            COMMAND "${CMAKE_COMMAND}" -E tar cf "${SILKIT_SYMBOLS_DIR_NAME}.zip" --format=zip -- "${SILKIT_SYMBOLS_DIR_NAME}"
            COMMENT "SIL Kit: SILKIT_PACKAGE_SYMBOLS: creating ELF debug symbols zip"
        )
    endif()
endmacro()

macro(silkit_enable_lto enableLto)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _lto_is_supported OUTPUT error)

    message("foo = ${enableLto} ${_enable} ${_lto_is_supported}")
    if (${enableLto} AND  _lto_is_supported)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

            find_program(LLD_EXECUTABLE NAMES lld ld.lld)
            if(LLD_EXECUTABLE)
                message("SIL Kit: Found LLD linker: ${LLD_EXECUTABLE}")
                # Test if clang can use LLD
                execute_process(COMMAND ${CMAKE_CXX_COMPILER} -fuse-ld=lld --version 
                    RESULT_VARIABLE LLD_TEST_RESULT OUTPUT_QUIET ERROR_QUIET)
                if(LLD_TEST_RESULT EQUAL 0)
                    message("SIL Kit: Using LLD linker for LTO")
                    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
                    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
                else()
                    message(WARNING "SIL Kit: LLD found but not compatible with this Clang version")
                endif()
            else()
                # Fallback to Gold linker
                find_program(GOLD_EXECUTABLE NAMES gold ld.gold)
                if(GOLD_EXECUTABLE)
                    message("SIL Kit: LLD not found, using Gold linker: ${GOLD_EXECUTABLE}")
                    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -fuse-ld=gold --version 
                        RESULT_VARIABLE GOLD_TEST_RESULT OUTPUT_QUIET ERROR_QUIET)
                    if(GOLD_TEST_RESULT EQUAL 0)
                        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
                        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold")
                    else()
                        message(WARNING "SIL Kit: Gold linker found but not compatible with this Clang version")
                    endif()
                else()
                    message(WARNING "SIL Kit: Neither LLD nor Gold linker found - using default linker (may cause LTO issues)")
                endif()
            endif()
        endif()
        message(STATUS "SIL Kit: Enabling Link Time Optimization globally.")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "SIL Kit: Disabling Link Time Optimization.")
    endif()

endmacro()

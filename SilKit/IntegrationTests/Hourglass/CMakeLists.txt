# SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(
    S_Test_Hourglass
    STATIC
    MockCapi.hpp
    MockCapi.cpp
)

target_include_directories(
    S_Test_Hourglass
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(
    S_Test_Hourglass
    PRIVATE EXPORT_SilKitAPI
)

target_link_libraries(
    S_Test_Hourglass
    PUBLIC SilKitInterface
    PRIVATE gtest
    PRIVATE gmock
)

if (MSVC)
    target_compile_options(S_Test_Hourglass PRIVATE /bigobj)
endif ()


add_silkit_test_executable(SilKitHourglassTests)

target_compile_definitions(SilKitHourglassTests
    PRIVATE SIL_KIT_TEST_MAIN_DO_NOT_PRINT_VERSION
)


add_silkit_test_to_executable(SilKitHourglassTests
    SOURCES 
        Test_HourglassParticipantLogger.cpp
    LIBS 
        S_Test_Hourglass
)

add_silkit_test_to_executable(SilKitHourglassTests
    SOURCES
        Test_HourglassCan.cpp
    LIBS
        S_Test_Hourglass
)

add_silkit_test_to_executable(SilKitHourglassTests
    SOURCES
        Test_HourglassEthernet.cpp
    LIBS
        S_Test_Hourglass
)

add_silkit_test_to_executable(SilKitHourglassTests
    SOURCES
        Test_HourglassLin.cpp
    LIBS
        S_Test_Hourglass
)

add_silkit_test_to_executable(SilKitHourglassTests
    SOURCES
        Test_HourglassFlexray.cpp
    LIBS
        S_Test_Hourglass
)

add_silkit_test_to_executable(SilKitHourglassTests
    SOURCES
        Test_HourglassRpc.cpp
    LIBS
        S_Test_Hourglass
)

add_silkit_test_to_executable(SilKitHourglassTests
    SOURCES
        Test_HourglassPubSub.cpp
    LIBS
        S_Test_Hourglass
)

add_silkit_test_to_executable(SilKitHourglassTests
    SOURCES
        Test_HourglassOrchestration.cpp
    LIBS
        S_Test_Hourglass
)

add_silkit_test_to_executable(SilKitHourglassTests
    SOURCES
        Test_HourglassVendor.cpp
    LIBS
        S_Test_Hourglass
)

add_silkit_test_to_executable(SilKitHourglassTests
    SOURCES
        Test_HourglassVersion.cpp
    LIBS
        S_Test_Hourglass
)

add_silkit_test_to_executable(SilKitHourglassTests
    SOURCES
        Test_HourglassNetSim.cpp
    LIBS
        S_Test_Hourglass
)

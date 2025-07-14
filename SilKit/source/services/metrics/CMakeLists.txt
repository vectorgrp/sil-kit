# SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT


add_library(I_SilKit_Services_Metrics INTERFACE)

target_include_directories(I_SilKit_Services_Metrics
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_Metrics
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Core_Internal
)


add_library(O_SilKit_Services_Metrics OBJECT
    MetricsDatatypes.cpp
    MetricsManager.cpp
    MetricsProcessor.cpp
    MetricsReceiver.cpp
    MetricsSender.cpp
    MetricsSerdes.cpp

    MetricsJsonSink.cpp
    MetricsRemoteSink.cpp

    MetricsTimerThread.cpp

    CreateMetricsSinksFromParticipantConfiguration.cpp

)

target_link_libraries(O_SilKit_Services_Metrics
    PUBLIC I_SilKit_Services_Metrics
    PRIVATE I_SilKit_Util_StringHelpers
)


add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_MetricsManager.cpp
    LIBS S_SilKitImpl
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_MetricsProcessor.cpp
    LIBS S_SilKitImpl
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_MetricsSerdes.cpp
    LIBS S_SilKitImpl
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_MetricsJsonSink.cpp
    LIBS S_SilKitImpl
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_MetricsRemoteSink.cpp
    LIBS S_SilKitImpl
)

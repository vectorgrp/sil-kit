# SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

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
    PRIVATE I_SilKit
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

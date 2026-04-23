# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(O_SilKit_Services_Orchestration OBJECT
    ILifecycleStates.hpp
    LifecycleManagement.hpp
    LifecycleManagement.cpp
    LifecycleStates.hpp
    LifecycleStates.cpp
    LifecycleService.hpp
    LifecycleService.cpp

    SyncDatatypeUtils.hpp
    SyncDatatypeUtils.cpp

    SystemController.hpp
    SystemController.cpp

    SystemMonitor.hpp
    SystemMonitor.cpp
    WatchDog.hpp
    WatchDog.cpp
    TimeSyncService.hpp
    TimeSyncService.cpp

    SyncSerdes.hpp
    SyncSerdes.cpp

    TimeProvider.hpp
    TimeProvider.cpp

    TimeConfiguration.hpp
    TimeConfiguration.cpp

    SystemStateTracker.cpp
)

target_link_libraries(O_SilKit_Services_Orchestration
    PRIVATE I_SilKit
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_LifecycleService.cpp 
    LIBS S_SilKitImpl I_SilKit
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_SystemController.cpp 
    LIBS S_SilKitImpl I_SilKit
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_SystemMonitor.cpp 
    LIBS S_SilKitImpl I_SilKit
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_WatchDog.cpp
    LIBS S_SilKitImpl
)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_SyncSerdes.cpp LIBS S_SilKitImpl I_SilKit)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_TimeProvider.cpp LIBS S_SilKitImpl I_SilKit)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_TimeSyncService.cpp LIBS S_SilKitImpl I_SilKit)

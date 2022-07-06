# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitMwSync
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
    TimeProvider.hpp
    TimeSyncService.hpp
    TimeSyncService.cpp
    
    SyncSerdes.hpp
    SyncSerdes.cpp
)

target_include_directories(SilKitMwSync
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(SilKitMwSync
    PUBLIC 
        SilKitInterface
        SilKitUtil
        SilKitSetThreadName
        SilKitInternal
    PRIVATE 
        SilKitInternal
        SilKitMwLogging
        SilKitConfiguration
    INTERFACE
        SilKitInternal
        SilKitConfiguration
)

add_silkit_test(TestMwLifecycleService
    SOURCES LifecycleServiceTest.cpp 
    LIBS SilKitMwSync SilKitMwMockParticipant
)
add_silkit_test(TestMwSystemController 
    SOURCES SystemControllerTest.cpp 
    LIBS SilKitMwSync SilKitMwMockParticipant
)
add_silkit_test(TestMwSystemMonitor 
    SOURCES SystemMonitorTest.cpp 
    LIBS SilKitMwSync SilKitMwMockParticipant
)
add_silkit_test(TestMwWatchDog
    SOURCES WatchDogTest.cpp
    LIBS SilKitMwSync
)
add_silkit_test(TestMwSync_Serdes SOURCES SyncSerdesTest.cpp LIBS SilKitMwSync SilKitInternal)

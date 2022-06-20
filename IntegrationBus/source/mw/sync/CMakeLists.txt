# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbMwSync
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

target_include_directories(IbMwSync
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(IbMwSync
    PUBLIC 
        IbInterface
        IbUtil
        IbSetThreadName
        IbInternal
    PRIVATE 
        IbInternal
        IbMwLogging
        IbConfiguration
    INTERFACE
        IbInternal
        IbConfiguration
)

add_vib_test(TestMwLifecycleService
    SOURCES LifecycleServiceTest.cpp 
    LIBS IbMwSync IbMwMockParticipant
)
add_vib_test(TestMwSystemController 
    SOURCES SystemControllerTest.cpp 
    LIBS IbMwSync IbMwMockParticipant
)
add_vib_test(TestMwSystemMonitor 
    SOURCES SystemMonitorTest.cpp 
    LIBS IbMwSync IbMwMockParticipant
)
add_vib_test(TestMwWatchDog
    SOURCES WatchDogTest.cpp
    LIBS IbMwSync
)
add_vib_test(TestMwSync_Serdes SOURCES SyncSerdesTest.cpp LIBS IbMwSync IbInternal)

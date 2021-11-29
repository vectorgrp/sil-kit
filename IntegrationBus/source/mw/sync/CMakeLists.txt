# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbMwSync
    ParticipantController.hpp
    ParticipantController.cpp
    SyncDatatypeUtils.hpp
    SyncDatatypeUtils.cpp
    SyncMaster.hpp
    SyncMaster.cpp
    SystemController.hpp
    SystemController.cpp
    SystemMonitor.hpp
    SystemMonitor.cpp
    WatchDog.hpp
    WatchDog.cpp
    TimeProvider.hpp
)

target_include_directories(IbMwSync
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(IbMwSync
    PUBLIC IbInterface
           IbUtil
           IbSetThreadName
    PUBLIC IbInternal
    PRIVATE IbMwLogging
)

add_vib_test(TestMwSyncMaster 
    SOURCES SyncMasterTest.cpp 
    LIBS IbMwSync IbMwMockComAdapter IbConfig )
add_vib_test(TestMwParticipantController 
    SOURCES ParticipantControllerTest.cpp 
    LIBS IbMwSync IbMwMockComAdapter IbConfig )
add_vib_test(TestMwSystemController 
    SOURCES SystemControllerTest.cpp 
    LIBS IbMwSync IbMwMockComAdapter)
add_vib_test(TestMwSystemMonitor 
    SOURCES SystemMonitorTest.cpp 
    LIBS IbMwSync IbMwMockComAdapter IbConfig)
add_vib_test(TestMwWatchDog
    SOURCES WatchDogTest.cpp
    LIBS IbMwSync IbConfig
)

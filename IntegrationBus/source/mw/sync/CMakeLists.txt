# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbMwSync
    ParticipantController.hpp
    ParticipantController.cpp
    SyncDatatypeUtils.hpp
    SyncDatatypeUtils.cpp
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
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(IbMwSync
    PUBLIC 
        IbInterface
        IbUtil
        IbSetThreadName
        IbInternal
    PRIVATE 
        IbMwLogging
        IbConfiguration
    INTERFACE
        IbConfiguration
)

add_vib_test(TestMwParticipantController 
    SOURCES ParticipantControllerTest.cpp 
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

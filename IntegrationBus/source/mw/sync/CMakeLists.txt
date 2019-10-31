add_library("IbMwSync"
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
)

target_include_directories("IbMwSync"
    PUBLIC .
)

target_link_libraries("IbMwSync"
    PUBLIC IbInterface
    PUBLIC IbUtil
    IbMwLogging
)

add_vib_test(TestMwSyncMaster SOURCES SyncMasterTest.cpp LIBS IbMwSync IbMwMockComAdapter IbConfig )
add_vib_test(TestMwParticipantController SOURCES ParticipantControllerTest.cpp LIBS IbMwSync IbMwMockComAdapter IbConfig )
add_vib_test(TestMwSystemController SOURCES SystemControllerTest.cpp LIBS IbMwSync IbMwMockComAdapter)
add_vib_test(TestMwSystemMonitor SOURCES SystemMonitorTest.cpp LIBS IbMwSync IbMwMockComAdapter IbConfig)
add_vib_test(TestMwWatchDog
    SOURCES WatchDogTest.cpp
    LIBS IbMwSync IbConfig
)

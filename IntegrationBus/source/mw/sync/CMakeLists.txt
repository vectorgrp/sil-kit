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
)

target_include_directories("IbMwSync"
    PUBLIC .
)

target_link_libraries("IbMwSync"
    PUBLIC IbInterface
    PUBLIC IbUtil
)

add_mvsc_ib_include_filter()

make_gtest(TestMwSyncMaster SyncMasterTest.cpp IbMwSync IbMwMock IbConfig)
make_gtest(TestMwParticipantController ParticipantControllerTest.cpp IbMwSync IbMwMock IbConfig)
make_gtest(TestMwSystemController SystemControllerTest.cpp IbMwSync IbMwMock)
make_gtest(TestMwSystemMonitor SystemMonitorTest.cpp IbMwSync IbMwMock IbConfig)

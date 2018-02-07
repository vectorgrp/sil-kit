project("IbMwSync" CXX)

add_library(${PROJECT_NAME}
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

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE IbInterface
)

add_mvsc_ib_include_filter()

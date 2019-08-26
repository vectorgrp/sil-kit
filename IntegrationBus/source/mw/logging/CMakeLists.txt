project("IbMwLogging" CXX)

add_library(${PROJECT_NAME}
    IIbToLogMsgDistributor.hpp
    IIbToLogMsgReceiver.hpp
    LogMsgDistributor.hpp
    LogMsgDistributor.cpp
    LogMsgReceiver.hpp
    LogMsgReceiver.cpp
    SpdlogTypeConversion.hpp
    Logger.hpp
    Logger.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC IbInterface
    PUBLIC spdlog
)

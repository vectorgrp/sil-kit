project("IbMwLogging" CXX)

add_library(${PROJECT_NAME}
    IIbToLogMsgSender.hpp
    IIbToLogMsgReceiver.hpp
    LogMsgSender.hpp
    LogMsgSender.cpp
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

add_vib_test(TestMwLoggingLogger SOURCES LoggerTest.cpp LIBS IbMwLogging IbConfig IbMwMockComAdapter)

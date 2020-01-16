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

if (TARGET fmt-header-only)
    message(STATUS "have fmt-header-only")
    target_link_libraries(${PROJECT_NAME}
        INTERFACE
            $<BUILD_INTERFACE:fmt-header-only>
        PRIVATE
            $<BUILD_INTERFACE:fmt-header-only>
    )
    target_compile_definitions(${PROJECT_NAME}
        INTERFACE
            $<BUILD_INTERFACE:HAVE_FMTLIB>
            $<BUILD_INTERFACE:FMT_HEADER_ONLY>
        PRIVATE
            $<BUILD_INTERFACE:HAVE_FMTLIB>
            $<BUILD_INTERFACE:FMT_HEADER_ONLY>
    )
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC IbInterface
    PUBLIC spdlog
)

add_vib_test(TestMwLoggingLogger SOURCES LoggerTest.cpp LIBS IbMwLogging IbConfig IbMwMockComAdapter)

project("IbMwLogging" CXX)

add_library(${PROJECT_NAME} STATIC
    SpdlogTypeConversion.hpp
    IIbToLogMsgSender.hpp
    IIbToLogMsgReceiver.hpp

    LogMsgSender.hpp
    LogMsgSender.cpp

    LogMsgReceiver.hpp
    LogMsgReceiver.cpp

    Logger.hpp
    Logger.cpp
    #string formatting for VIB types
    VibFmtFormatters.hpp
)
################################################################################
# Thirdparty -- Logging
################################################################################
# We use fmtlib to offer a convenient logging interface

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
    INTERFACE ${PROJECT_SOURCE_DIR}
)


target_link_libraries(${PROJECT_NAME}
    PUBLIC IbInterface
    PRIVATE spdlog
)

if(UNIX)
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
        VISIBILITY_INLINES_HIDDEN 1
        CXX_VISIBILITY_PRESET hidden
    )
endif()

add_vib_test(TestMwLoggingLogger SOURCES LoggerTest.cpp LIBS IbMwLogging IbConfig IbMwMockComAdapter)

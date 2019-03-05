project("IbMwLogging" CXX)

add_library(${PROJECT_NAME}
    ILogmsgRouter.hpp
    IIbToLogmsgRouter.hpp
    LogmsgRouter.hpp
    LogmsgRouter.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE IbInterface
)

add_mvsc_ib_include_filter()

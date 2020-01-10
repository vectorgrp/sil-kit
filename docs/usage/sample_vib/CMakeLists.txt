Project(SampleVib)
cmake_minimum_required(VERSION 3.0)
set(CMAKE_BUILD_TYPE Debug)
set(VIB_DIR ${CMAKE_CURRENT_LIST_DIR}/IntegrationBus)
find_package(IntegrationBus
    REQUIRED
    CONFIG
    NO_CMAKE_PACKAGE_REGISTRY
    NO_DEFAULT_PATH
    PATHS "${VIB_DIR}"
)
add_executable(${PROJECT_NAME} simple.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE IntegrationBus::IntegrationBus)

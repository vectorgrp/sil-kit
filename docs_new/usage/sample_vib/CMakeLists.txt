# Copyright (c) Vector Informatik GmbH. All rights reserved.
Project(SampleVib)
cmake_minimum_required(VERSION 3.0)
set(CMAKE_BUILD_TYPE Debug)
find_package(IntegrationBus 
    REQUIRED CONFIG
    PATHS "${CMAKE_CURRENT_LIST_DIR}/IntegrationBus"
)
add_executable(${PROJECT_NAME} simple.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE IntegrationBus::IntegrationBus)

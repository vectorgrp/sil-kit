# Copyright (c) Vector Informatik GmbH. All rights reserved.
Project(SampleVib)
cmake_minimum_required(VERSION 3.0)
set(CMAKE_BUILD_TYPE Debug)
find_package(SilKit 
    REQUIRED CONFIG
    PATHS "${CMAKE_CURRENT_LIST_DIR}/SilKit"
)
find_package(Threads REQUIRED)
add_executable(${PROJECT_NAME} simple.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE SilKit::SilKit Threads::Threads)

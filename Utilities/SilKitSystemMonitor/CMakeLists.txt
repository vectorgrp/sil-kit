# Copyright (c) Vector Informatik GmbH. All rights reserved.
cmake_minimum_required(VERSION 3.8)
project("SilKit-SystemMonitor" LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)

include(SilKitInstall)

add_executable(sil-kit-monitor
    PassiveSystemMonitor.cpp
)

# Group this demo project into a folder
set_target_properties(sil-kit-monitor PROPERTIES
     FOLDER "Utilities"
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     INSTALL_RPATH "$ORIGIN/../lib"
)

target_link_libraries(sil-kit-monitor
    SilKit
    SilKitUtil
)
install(TARGETS sil-kit-monitor
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)

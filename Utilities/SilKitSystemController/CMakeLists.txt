# Copyright (c) Vector Informatik GmbH. All rights reserved.
cmake_minimum_required(VERSION 3.8)
project("sil-kit-system-controller" LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)

include(SilKitInstall)

add_executable(sil-kit-system-controller
    SystemController.cpp
)

# Group this demo project into a folder
set_target_properties(sil-kit-system-controller PROPERTIES
    FOLDER "Utilities"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    INSTALL_RPATH "$ORIGIN/../lib"
)

target_link_libraries(sil-kit-system-controller
    SilKit
    SilKitUtil
)
install(TARGETS sil-kit-system-controller
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)

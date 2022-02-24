# Copyright (c) Vector Informatik GmbH. All rights reserved.
cmake_minimum_required(VERSION 3.8)
project(IbConfigTool)

set(CMAKE_CXX_STANDARD 14)

include(IntegrationBusInstall)

set(EXE vib-config-tool)
add_executable(${EXE}
    IbConfigTool.cpp
)

# Group this demo project into a folder
set_target_properties(${EXE} PROPERTIES
     FOLDER "Utilities"
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     INSTALL_RPATH "$ORIGIN/../lib"
)

# legacy json parser moved here from IntegrationBus/source/cfg/
# we keep this for backwards compatibility:
# the YAML parser doesn't support some legacy fixups.
target_link_libraries(${EXE}
    PRIVATE
    IbInterface
)
install(TARGETS ${EXE}
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)

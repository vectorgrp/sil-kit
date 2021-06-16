cmake_minimum_required(VERSION 3.8)
project(IbValidateConfig)

set(CMAKE_CXX_STANDARD 14)

include(IntegrationBusInstall)

add_executable(IbValidateConfig
	ValidateConfig.cpp
)

# Group this demo project into a folder
set_target_properties(IbValidateConfig PROPERTIES
     FOLDER "Utilities"
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     INSTALL_RPATH "$ORIGIN/../lib"
)

target_link_libraries(IbValidateConfig
    IntegrationBus
)
install(TARGETS IbValidateConfig
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)

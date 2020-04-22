cmake_minimum_required(VERSION 3.8)
project("IntegrationBus-SystemController" LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)

include(IntegrationBusInstall)

add_executable(IbSystemController
    SystemController.cpp
)

# Group this demo project into a folder
set_target_properties(IbSystemController PROPERTIES
     FOLDER "Utilities"
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

target_link_libraries(IbSystemController
    IntegrationBus
)
install(TARGETS IbSystemController
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)

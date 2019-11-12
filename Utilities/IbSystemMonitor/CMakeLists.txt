cmake_minimum_required(VERSION 3.8)
project("IntegrationBus-SystemMonitor" LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)

include(IntegrationBusInstall)

add_executable(IbSystemMonitor
    PassiveSystemMonitor.cpp
)

# Group this demo project into a folder
set_target_properties(IbSystemMonitor PROPERTIES
     FOLDER "Utilities"
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Utilities/$<CONFIG>/bin"
)

target_link_libraries(IbSystemMonitor
    IntegrationBus
)
install(TARGETS IbSystemMonitor
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)

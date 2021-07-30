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
add_library(IbJsonParser STATIC
    JsonConfig.hpp
    JsonConfig.cpp
    )
target_link_libraries(IbJsonParser
    PRIVATE 
    json11
    IbInterface
    INTERFACE 
    json11
    )

target_include_directories(IbJsonParser 
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../IntegrationBus/source/cfg
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/../../IntegrationBus/source/cfg
)
target_link_libraries(${EXE}
    PRIVATE
    IbInterface
    IbConfig
    IbJsonParser
)
install(TARGETS ${EXE}
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)


add_vib_test(TestMwCfgJsonConfig SOURCES JsonConfigTest.cpp LIBS IbJsonParser IntegrationBus )
ib_enable_warnings(TestMwCfgJsonConfig)

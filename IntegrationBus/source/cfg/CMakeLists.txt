add_library(json11
    ${THIRDPARTY_DIR}/json11/json11.hpp
    ${THIRDPARTY_DIR}/json11/json11.cpp
    )

target_include_directories(json11
    PUBLIC ${THIRDPARTY_DIR}/json11
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET json11 PROPERTY FOLDER "ThirdParty")


add_library(IbConfig STATIC
    Config.cpp
    ConfigBuilder.cpp
    ControllerBuilder.cpp
    LoggerBuilder.cpp
    GenericPortBuilder.cpp
    LinkBuilder.cpp
    NetworkSimulatorBuilder.cpp
    ParticipantBuilder.cpp
    SimulationSetupBuilder.cpp
    SwitchBuilder.cpp
    SwitchPortBuilder.cpp
    TimeSyncBuilder.cpp
    FastRtpsConfigBuilder.cpp
    JsonConfig.hpp
    JsonConfig.cpp
    Validation.hpp
    Validation.cpp
    VAsioConfigBuilder.cpp
    VAsioConfigBuilder.hpp
    VAsioRegistryBuilder.cpp
    VAsioRegistryBuilder.hpp
)

target_link_libraries(IbConfig
    PRIVATE IbInterface
    PRIVATE IbUtil
    PRIVATE json11
)

target_include_directories(IbConfig
    PUBLIC .
)

add_vib_test(TestMwCfgConfigBuilder SOURCES ConfigBuilderTest.cpp LIBS IbConfig)
add_vib_test(TestMwCfgJsonConfig SOURCES JsonConfigTest.cpp LIBS IbConfig json11)
add_vib_test(TestMwCfgValidation SOURCES ValidationTest.cpp LIBS IbConfig)
add_vib_test(TestIbConfigExample SOURCES IbConfigExampleTest.cpp
    LIBS IbMwMockNullConnection
    CONFIGS
        IbConfig_Example.json
        IbConfig_IO-Example.json
        IbConfig_Canoe.json
        IbConfig_FastRTPS-local.json
        IbConfig_FastRTPS-multicast.json
        IbConfig_FastRTPS-unicast.json
        IbConfig_FastRTPS-configfile.json
)

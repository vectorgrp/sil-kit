# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbConfig_deprecated STATIC
    Config.cpp
    Config.hpp
    ConfigBuilder.cpp
    ConfigBuilder.hpp
    ControllerBuilder.cpp
    LoggerBuilder.hpp
    LoggerBuilder.cpp
    SinkBuilder.hpp
    SinkBuilder.cpp
    GenericPortBuilder.cpp
    GenericPortBuilder.hpp
    DataPortBuilder.cpp
    DataPortBuilder.hpp
    RpcPortBuilder.cpp
    RpcPortBuilder.hpp
    LinkBuilder.cpp
    LinkBuilder.hpp
    NetworkSimulatorBuilder.cpp
    ParticipantBuilder.cpp
    ParticipantBuilder.hpp
    ParticipantControllerBuilder.cpp
    ParticipantControllerBuilder.hpp
    SimulationSetupBuilder.cpp
    SimulationSetupBuilder.hpp
    SwitchBuilder.cpp
    SwitchBuilder.hpp
    SwitchPortBuilder.cpp
    SwitchPortBuilder.hpp
    TimeSyncBuilder.cpp
    TimeSyncBuilder.hpp
    Validation.hpp
    Validation.cpp
    VAsioConfigBuilder.cpp
    VAsioConfigBuilder.hpp
    VAsioRegistryBuilder.cpp
    VAsioRegistryBuilder.hpp
    ExtensionConfigBuilder.cpp
    ExtensionConfigBuilder.hpp
    TraceSinkBuilder.hpp
    TraceSinkBuilder.cpp
    TraceSourceBuilder.hpp
    TraceSourceBuilder.cpp
    ReplayBuilder.hpp
    ReplayBuilder.cpp

    YamlConfig.hpp
    YamlConfig.cpp
    YamlConfigConversion.cpp
    YamlConfigConversion.hpp
    YamlSchema.hpp
    YamlSchema.cpp
    YamlValidator.hpp
    YamlValidator.cpp
)

target_link_libraries(IbConfig_deprecated
    PRIVATE IbInterface
    PRIVATE IbUtil
    PRIVATE IbSimDataMessage
    PRIVATE yaml-cpp
    INTERFACE yaml-cpp
)

target_include_directories(IbConfig_deprecated
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
)

add_vib_test(TestMwCfgConfigBuilder_deprecated SOURCES ConfigBuilderTest.cpp LIBS IbConfig_deprecated)
add_vib_test(TestMwCfgValidation_deprecated SOURCES ValidationTest.cpp LIBS IbConfig_deprecated)
add_vib_test(TestIbConfigExample_deprecated SOURCES IbConfigExampleTest.cpp
    LIBS
        IbMwMockNullConnection 
        IbCfgMockParticipantConfiguration
    CONFIGS
        IbConfig_Example.json
        IbConfig_Bad_FileLogger.json
        IbConfig_Canoe.json
)

add_vib_test(TestMwCfgYamlConfig_deprecated SOURCES YamlConfigTest.cpp LIBS IbConfig_deprecated IbSimGenericMessage yaml-cpp)

add_vib_test(TestMwCfgJsonConfig_deprecated SOURCES JsonConfigTest.cpp LIBS IbConfig_deprecated)
# we need runtime IntegrationBus DLL on windows:
#set_tests_properties(TestMwCfgJsonConfig_deprecated PROPERTIES 
#    WORKING_DIRECTORY $<TARGET_FILE_DIR:IntegrationBus>)

ib_enable_warnings(IbConfig_deprecated)
ib_enable_warnings(TestMwCfgConfigBuilder_deprecated)
ib_enable_warnings(TestMwCfgValidation_deprecated)
ib_enable_warnings(TestMwCfgYamlConfig_deprecated)
ib_enable_warnings(TestMwCfgJsonConfig_deprecated)



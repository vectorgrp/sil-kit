set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Third party YAML parsing 
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Disable because of gmock import conflict" FORCE)
add_subdirectory(${THIRDPARTY_DIR}/yaml-cpp
	${CMAKE_BINARY_DIR}/ThirdParty/yaml-cpp
	EXCLUDE_FROM_ALL
	)
# ensure all external projects are grouped in IDE mode
set(yaml_targets yaml-cpp parse read sandbox format)
foreach(_tgt IN LISTS yaml_targets)
	set_property(TARGET ${_tgt} PROPERTY FOLDER "ThirdParty/yaml-cpp")
endforeach()


# Third Party JSON parsing
add_library(json11
    ${THIRDPARTY_DIR}/json11/json11.hpp
    ${THIRDPARTY_DIR}/json11/json11.cpp
    )

target_include_directories(json11
    PUBLIC ${THIRDPARTY_DIR}/json11
)

set_property(TARGET json11 PROPERTY FOLDER "ThirdParty")


add_library(IbConfig STATIC
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
    FastRtpsConfigBuilder.cpp
    FastRtpsConfigBuilder.hpp
    JsonConfig.hpp
    JsonConfig.cpp
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

target_link_libraries(IbConfig
    PRIVATE IbInterface
    PRIVATE IbUtil
    PRIVATE json11
	PRIVATE yaml-cpp
)

target_include_directories(IbConfig
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
)

add_vib_test(TestMwCfgConfigBuilder SOURCES ConfigBuilderTest.cpp LIBS IbConfig)
add_vib_test(TestMwCfgJsonConfig SOURCES JsonConfigTest.cpp LIBS IbConfig json11)
add_vib_test(TestMwCfgValidation SOURCES ValidationTest.cpp LIBS IbConfig)
add_vib_test(TestIbConfigExample SOURCES IbConfigExampleTest.cpp
    LIBS IbMwMockNullConnection
    CONFIGS
        IbConfig_Example.json
        IbConfig_Bad_FileLogger.json
        IbConfig_IO-Example.json
        IbConfig_Canoe.json
        IbConfig_FastRTPS-local.json
        IbConfig_FastRTPS-multicast.json
        IbConfig_FastRTPS-unicast.json
        IbConfig_FastRTPS-configfile.json
)

add_vib_test(TestMwCfgYamlConfig SOURCES YamlConfigTest.cpp LIBS IbConfig yaml-cpp)

ib_enable_warnings(IbConfig)
ib_enable_warnings(TestMwCfgJsonConfig)
ib_enable_warnings(TestMwCfgConfigBuilder)
ib_enable_warnings(TestMwCfgValidation)
ib_enable_warnings(TestMwCfgYamlConfig)



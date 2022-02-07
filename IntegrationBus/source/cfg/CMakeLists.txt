# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbConfiguration STATIC
    Configuration.hpp
    Configuration.cpp
    ParticipantConfiguration.hpp
    ParticipantConfiguration.cpp
    vasio/MiddlewareConfiguration.hpp
    vasio/MiddlewareConfiguration.cpp
#    YamlConfig.hpp
#    YamlConfig.cpp
#    YamlConfigConversion.hpp
#    YamlConfigConversion.cpp
#    YamlSchema.hpp
#    YamlSchema.cpp
#    YamlValidator.hpp
#    YamlValidator.cpp
)

target_link_libraries(IbConfiguration
    PRIVATE IbInterface
    PRIVATE IbUtil
    INTERFACE IbUtil
    PRIVATE yaml-cpp
    INTERFACE yaml-cpp
)

target_include_directories(IbConfiguration
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

#add_vib_test(TestMwCfgConfigBuilder 
#    SOURCES ConfigBuilderTest.cpp 
#    LIBS IbConfiguration
#)
#add_vib_test(TestMwCfgValidation 
#    SOURCES ValidationTest.cpp 
#    LIBS IbConfiguration
#)
#add_vib_test(TestIbConfigExample 
#    SOURCES IbConfigExampleTest.cpp
#    LIBS IbMwMockNullConnection
#    CONFIGS
#        IbConfig_Example.json
#        IbConfig_Bad_FileLogger.json
#        IbConfig_Canoe.json
#)

#add_vib_test(TestMwCfgYamlConfig 
#    SOURCES YamlConfigTest.cpp 
#    LIBS IbConfig IbSimGenericMessage yaml-cpp
#)

#add_vib_test(TestMwCfgJsonConfig 
#    SOURCES JsonConfigTest.cpp 
#    LIBS IbConfiguration
#)
## we need runtime IntegrationBus DLL on windows:
##set_tests_properties(TestMwCfgJsonConfig PROPERTIES 
##    WORKING_DIRECTORY $<TARGET_FILE_DIR:IntegrationBus>)

ib_enable_warnings(IbConfiguration)
#ib_enable_warnings(TestMwCfgConfigBuilder)
#ib_enable_warnings(TestMwCfgValidation)
#ib_enable_warnings(TestMwCfgYamlConfig)
#ib_enable_warnings(TestMwCfgJsonConfig)



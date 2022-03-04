# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbConfiguration STATIC
    Configuration.hpp
    Configuration.cpp
    ParticipantConfiguration.hpp
    ParticipantConfiguration.cpp
    Validation.hpp
    Validation.cpp
    YamlParser.hpp
    YamlParser.cpp
    YamlConversion.hpp
    YamlConversion.cpp
    YamlSchema.hpp
    YamlSchema.cpp
    YamlValidator.hpp
    YamlValidator.cpp
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

add_vib_test(TestMwCfgValidation 
    SOURCES ValidationTest.cpp 
    LIBS IbConfiguration
)

add_vib_test(TestParticipantConfiguration 
    SOURCES ParticipantConfigurationTest.cpp
    LIBS IbMwMockNullConnection
    CONFIGS
        ParticipantConfiguration_Minimal.json
        ParticipantConfiguration_Full.json
        ParticipantConfiguration_Logging_Without_File.json
)

add_vib_test(TestMwCfgYamlParser 
    SOURCES YamlParserTest.cpp 
    LIBS IbConfiguration yaml-cpp
)

add_vib_test(TestMwCfgYamlValidator 
    SOURCES YamlValidatorTest.cpp 
    LIBS IbConfiguration yaml-cpp
)

add_subdirectory(mock)

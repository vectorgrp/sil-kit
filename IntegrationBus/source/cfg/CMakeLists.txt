# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitConfiguration STATIC
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

target_link_libraries(SilKitConfiguration
    PRIVATE SilKitInterface
    PRIVATE SilKitUtil
    INTERFACE SilKitUtil
    PRIVATE yaml-cpp
    INTERFACE yaml-cpp
)

target_include_directories(SilKitConfiguration
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_silkit_test(TestMwCfgValidation 
    SOURCES ValidationTest.cpp 
    LIBS SilKitConfiguration
)

add_silkit_test(TestParticipantConfiguration 
    SOURCES ParticipantConfigurationTest.cpp
    LIBS SilKitMwMockNullConnection
    CONFIGS
        ParticipantConfiguration_Minimal.json
        ParticipantConfiguration_Full.json
        ParticipantConfiguration_Logging_Without_File.json
)

add_silkit_test(TestMwCfgYamlParser 
    SOURCES YamlParserTest.cpp 
    LIBS SilKitConfiguration yaml-cpp
)

add_silkit_test(TestMwCfgYamlValidator 
    SOURCES YamlValidatorTest.cpp 
    LIBS SilKitConfiguration yaml-cpp
)

if(SILKIT_BUILD_TESTS)
    add_library(SilKitConfigurationTestUtils INTERFACE)

    target_include_directories(SilKitConfigurationTestUtils
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}
    )

    target_link_libraries(SilKitConfigurationTestUtils
        INTERFACE SilKitConfiguration
    )
endif()

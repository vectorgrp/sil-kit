# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Config INTERFACE)

target_include_directories(I_SilKit_Config
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(I_SilKit_Config
    INTERFACE
        ${SILKIT_THIRD_PARTY_YAML_INTERFACE}
        I_SilKit_Util
        rapidyaml
)

add_library(O_SilKit_Config OBJECT
    Configuration.hpp
    Configuration.cpp
    ParticipantConfiguration.hpp
    ParticipantConfiguration.cpp
    ParticipantConfigurationFromXImpl.hpp
    ParticipantConfigurationFromXImpl.cpp
    Validation.hpp
    Validation.cpp

    YamlParser.hpp
    YamlParser.cpp
    YamlReader.hpp
    YamlReader.cpp
    YamlWriter.hpp
    YamlWriter.cpp
    
    YamlValidator.hpp
    YamlValidator.cpp
)

target_link_libraries(O_SilKit_Config
    PUBLIC I_SilKit_Config

    PRIVATE SilKitInterface
    PRIVATE I_SilKit_Util_FileHelpers
    PUBLIC rapidyaml
    PRIVATE fmt::fmt
)

target_include_directories(O_SilKit_Config
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_Validation.cpp
    LIBS O_SilKit_Config S_SilKitImpl
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_ParticipantConfiguration.cpp
    LIBS S_SilKitImpl O_SilKit_Core_Mock_NullConnection
    CONFIGS
        ParticipantConfiguration_Minimal.json
        ParticipantConfiguration_Full.json
        ParticipantConfiguration_Full.yaml

        TestParticipantConfigs/ParticipantConfiguration_TestCapi.yaml
        TestParticipantConfigs/ParticipantConfiguration_FullIncludes.yaml
        TestParticipantConfigs/ParticipantConfiguration_FullIncludes_Reference.yaml

        TestParticipantConfigs/ExtensionsIncludes.yaml

        TestParticipantConfigs/LoggingIncludes.yaml

        TestParticipantConfigs/ParticipantConfiguration_MultipleAcceptorUris.yaml
        TestParticipantConfigs/MultipleAcceptorUrisInclude.yaml

        TestParticipantConfigs/ParticipantConfiguration_DuplicateControllerNames.yaml
        TestParticipantConfigs/DuplicateEthernetControllerNames.yaml

        TestParticipantConfigs/ParticipantConfiguration_FromString_Reference.yaml

        ParticipantConfiguration_Logging_Without_File.json
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_YamlParser.cpp
    LIBS O_SilKit_Config rapidyaml
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_YamlValidator.cpp
    LIBS O_SilKit_Config rapidyaml
)

set(ParticipantTestCanConfigs
    TestParticipantConfigs/CanIncludes.yaml
    TestParticipantConfigs/CanInclude2.yaml
    TestParticipantConfigs/CanInclude3.yaml
    TestParticipantConfigs/CanInclude4.yaml
    TestParticipantConfigs/CanInclude5.yaml
    TestParticipantConfigs/CanInclude6.yaml
)
set(ParticipantTestMiddlewareConfigs
    TestParticipantConfigs/MiddlewareInclude.yaml
    TestParticipantConfigs/MoreMiddlewareIncludes.yaml
)

add_custom_target(CopyTestConfigs)

function(copy_participant_configs CopyTarget Configs ConfigsDir)
    foreach(config ${Configs})
        get_filename_component(configPath "${config}" ABSOLUTE)
        list(APPEND TEST_CONFIGS "${configPath}")
    endforeach()

    add_custom_command(TARGET ${CopyTarget} POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_BINARY_DIR}/$<CONFIG>/ConfigSnippets/${ConfigsDir}
    )
    add_custom_command(TARGET ${CopyTarget} POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy
            "${TEST_CONFIGS}"
            ${CMAKE_BINARY_DIR}/$<CONFIG>/ConfigSnippets/${ConfigsDir}
        COMMAND_EXPAND_LISTS
    )
endfunction()

copy_participant_configs(CopyTestConfigs "${ParticipantTestCanConfigs}" "CAN")
copy_participant_configs(CopyTestConfigs "${ParticipantTestMiddlewareConfigs}" "Middleware")

if(SILKIT_BUILD_TESTS)
    add_dependencies(SilKitUnitTests CopyTestConfigs)

    add_library(I_SilKit_Config_TestUtils INTERFACE)

    target_include_directories(I_SilKit_Config_TestUtils
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}
    )
    target_link_libraries(I_SilKit_Config_TestUtils
        INTERFACE I_SilKit_Config
    )
endif()


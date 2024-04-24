# Copyright (c) 2022 Vector Informatik GmbH
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

add_library(I_SilKit_Config INTERFACE)

target_include_directories(I_SilKit_Config
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(I_SilKit_Config
    INTERFACE
        ${SILKIT_THIRD_PARTY_YAML_INTERFACE}
        I_SilKit_Util
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
    YamlConversion.hpp
    YamlConversion.cpp
    YamlSchema.hpp
    YamlSchema.cpp
    YamlValidator.hpp
    YamlValidator.cpp
)

target_link_libraries(O_SilKit_Config
    PUBLIC I_SilKit_Config

    PRIVATE SilKitInterface
    PRIVATE O_SilKit_Util_Filesystem
    PUBLIC yaml-cpp
    PRIVATE I_SilKit_Util_FileHelpers
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
    LIBS O_SilKit_Config yaml-cpp
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_YamlValidator.cpp
    LIBS O_SilKit_Config yaml-cpp
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


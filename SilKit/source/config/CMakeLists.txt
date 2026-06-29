# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

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

    CapabilitiesParser.hpp
    CapabilitiesParser.cpp

    YamlValidator.hpp
    YamlValidator.cpp
)

target_link_libraries(O_SilKit_Config
    PRIVATE I_SilKit
)


################################################################################
# SilKit::Yaml -- header-only auxiliary YAML-parsing library for out-of-tree
# projects (unstable API/ABI). Opt-in via -DSILKIT_INSTALL_YAML=ON.
#
# The generic YAML machinery lives in the public sibling folder
# SilKit/include/silkit_yaml/ and is header-only. rapidyaml is a single-header
# library; consumers compile its implementation exactly once via the bundled
# silkit_yaml/RapidyamlImpl.cpp. Nothing is compiled here -- the INTERFACE target
# only carries usage requirements.
################################################################################
if(SILKIT_INSTALL_YAML)
    set(_silkit_yaml_rapidyaml_dir "${SILKIT_THIRD_PARTY_SOURCE_DIR}/rapidyaml")

    add_library(SilKitYaml INTERFACE)
    add_library(SilKit::Yaml ALIAS SilKitYaml)
    # Export as SilKit::Yaml (matching the in-tree alias) rather than the default
    # SilKit::SilKitYaml derived from the target name.
    set_target_properties(SilKitYaml PROPERTIES EXPORT_NAME Yaml)

    target_include_directories(SilKitYaml
        INTERFACE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${_silkit_yaml_rapidyaml_dir}>
            $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}/silkit_yaml> # for unprefixed rapidyaml.hpp
    )
    target_link_libraries(SilKitYaml INTERFACE SilKitInterface) # for silkit/participant/exception.hpp
    target_compile_definitions(SilKitYaml INTERFACE RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS=1)
    target_compile_features(SilKitYaml INTERFACE cxx_std_17)

    # Install the silkit_yaml/ headers + the impl TU (excluded from the default
    # header glob in SilKit/include/CMakeLists.txt), the bundled rapidyaml header,
    # and the exported INTERFACE target.
    install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/include/silkit_yaml/
        DESTINATION ${INSTALL_INCLUDE_DIR}/silkit_yaml
        COMPONENT yaml
        FILES_MATCHING
            PATTERN "*.hpp"
            PATTERN "*.cpp"
            PATTERN "*.md"
    )
    install(
        FILES ${_silkit_yaml_rapidyaml_dir}/rapidyaml.hpp
        DESTINATION ${INSTALL_INCLUDE_DIR}/silkit_yaml
        COMPONENT yaml
    )
    install(
        TARGETS SilKitYaml
        EXPORT SilKitYamlTargets
        COMPONENT yaml
    )
    install(
        EXPORT SilKitYamlTargets
        DESTINATION ${INSTALL_LIB_DIR}/cmake/SilKit
        NAMESPACE "SilKit::"
        FILE SilKitYamlTargets.cmake
        COMPONENT yaml
    )
endif()


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
    LIBS O_SilKit_Config
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_YamlValidator.cpp
    LIBS O_SilKit_Config
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
endif()

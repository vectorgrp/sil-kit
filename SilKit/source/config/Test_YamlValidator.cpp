// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "config/YamlParser.hpp"
#include "config/YamlValidator.hpp"

#include "config/ParticipantConfiguration.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <fstream>

namespace {

class Test_YamlValidator : public testing::Test
{
};

using namespace SilKit::Config;

TEST_F(Test_YamlValidator, validate_without_warnings)
{
    ParticipantConfiguration cfg;
    cfg.schemaVersion = "1";
    cfg.middleware.enableDomainSockets = true;
    cfg.middleware.connectAttempts = 1234;
    cfg.middleware.registryUri = "silkit://not-localhost";
    cfg.middleware.tcpNoDelay = true;
    cfg.middleware.tcpQuickAck = true;
    cfg.middleware.tcpReceiveBufferSize = 1234;
    cfg.middleware.tcpSendBufferSize = 1234;

    std::stringstream stream;
    auto jsonString = SerializeAsJson(cfg);
    auto isValid = ValidateWithSchema(jsonString, stream);
    EXPECT_TRUE(isValid);
    auto warnings = stream.str();
    EXPECT_TRUE(warnings.empty()) << "Warnings: " << warnings;
}

TEST_F(Test_YamlValidator, validate_rejects_multi_document_stream)
{
    // A stray "---" mid-config splits it into two documents; the validator must reject it.
    auto yamlString = R"yaml(Description: Log to Stdout with Level Info
Logging:
---
  Sinks:
    - Level: Info
      Type: Stdout
)yaml";

    std::stringstream warnings;
    bool yamlValid = ValidateWithSchema(yamlString, warnings);
    EXPECT_FALSE(yamlValid) << "A multi-document stream must not validate";
    EXPECT_THAT(warnings.str(), testing::HasSubstr("one YAML document"));
}

TEST_F(Test_YamlValidator, validate_accepts_single_leading_document_marker)
{
    // A single leading "---" is one document and stays valid.
    auto yamlString = R"yaml(---
schemaVersion: 1
ParticipantName: CanDemoParticipant
)yaml";

    std::stringstream warnings;
    bool yamlValid = ValidateWithSchema(yamlString, warnings);
    EXPECT_TRUE(yamlValid) << "A single leading '---' document must validate. Warnings: "
                           << warnings.str();
    EXPECT_TRUE(warnings.str().empty()) << "Warnings: " << warnings.str();
}

TEST_F(Test_YamlValidator, validate_unknown_toplevel)
{
    auto yamlString = R"yaml(
schemaVersion: 1
ParticipantName: CanDemoParticipant
Description: Sample configuration for CAN
#typo in a toplevel statement, additional 's'
CanControllerss:
)yaml";

    std::stringstream warnings;
    bool yamlValid = ValidateWithSchema(yamlString, warnings);
    std::cout << "Yaml Validator warnings: " << warnings.str() << std::endl;
    EXPECT_TRUE(yamlValid) << "We ignore non-keyword errors and typos, but generate warnings!";
    EXPECT_GT(warnings.str().size(), 0u) << "Yaml Validator warnings: '" << warnings.str() << "'";
    // The warning must be complete: it names the field and the schema path it is ignored under
    EXPECT_THAT(warnings.str(), testing::HasSubstr("CanControllerss"));
    EXPECT_THAT(warnings.str(), testing::HasSubstr("is being ignored"));
    EXPECT_THAT(warnings.str(), testing::HasSubstr("schema path \"/\""));
}

TEST_F(Test_YamlValidator, validate_unknown_toplevel_arbitrary)
{
    auto yamlString = R"yaml(
schemaVersion: 1
ParticipantName: P1
# a completely unknown field, not a near-miss of any keyword
Foobar: true
)yaml";

    std::stringstream warnings;
    bool yamlValid = ValidateWithSchema(yamlString, warnings);
    std::cout << "Yaml Validator warnings: " << warnings.str() << std::endl;
    EXPECT_TRUE(yamlValid) << "Unknown fields are warned about, not rejected";
    EXPECT_THAT(warnings.str(), testing::HasSubstr("Foobar"));
    EXPECT_THAT(warnings.str(), testing::HasSubstr("is being ignored"));
    EXPECT_THAT(warnings.str(), testing::HasSubstr("schema path \"/\""));
}

TEST_F(Test_YamlValidator, validate_unknown_nested)
{
    auto yamlString = R"yaml(
schemaVersion: 1
ParticipantName: P1
Middleware:
  Foobar: true
CanControllers:
  - Name: CAN1
    Foobaz: 42
)yaml";

    std::stringstream warnings;
    bool yamlValid = ValidateWithSchema(yamlString, warnings);
    std::cout << "Yaml Validator warnings: " << warnings.str() << std::endl;
    EXPECT_TRUE(yamlValid) << "Unknown nested fields are warned about, not rejected";
    EXPECT_THAT(warnings.str(), testing::HasSubstr("Foobar"));
    EXPECT_THAT(warnings.str(), testing::HasSubstr("schema path \"/Middleware\""));
    EXPECT_THAT(warnings.str(), testing::HasSubstr("Foobaz"));
    EXPECT_THAT(warnings.str(), testing::HasSubstr("schema path \"/CanControllers\""));
}

TEST_F(Test_YamlValidator, validate_logging_sink_experimental)
{
    // Regression: the reader reads EnabledTopics/DisabledTopics under a nested Experimental
    // node, so the schema must accept that nesting without warnings.
    auto yamlString = R"yaml(
schemaVersion: 1
ParticipantName: P1
Logging:
  Sinks:
  - Type: Remote
    Experimental:
      EnabledTopics:
      - TopicA
      DisabledTopics:
      - TopicB
)yaml";

    std::stringstream warnings;
    bool yamlValid = ValidateWithSchema(yamlString, warnings);
    EXPECT_TRUE(yamlValid) << "YamlValidator warnings: " << warnings.str();
    EXPECT_EQ(warnings.str(), "") << "Nested Sink Experimental topics must validate cleanly";
}

TEST_F(Test_YamlValidator, validate_rpc_deprecated_channel_alias)
{
    // Regression: deprecated but still-supported RPC keys Channel/RpcChannel must not warn.
    auto yamlString = R"yaml(
schemaVersion: 1
ParticipantName: P1
RpcServers:
  - Name: Server1
    Channel: FuncA
RpcClients:
  - Name: Client1
    RpcChannel: FuncA
)yaml";

    std::stringstream warnings;
    bool yamlValid = ValidateWithSchema(yamlString, warnings);
    EXPECT_TRUE(yamlValid) << "YamlValidator warnings: " << warnings.str();
    EXPECT_EQ(warnings.str(), "") << "Deprecated RPC Channel aliases must validate cleanly";
}

TEST_F(Test_YamlValidator, validate_duplicate_element)
{
    const auto yamlString = R"raw(
LinControllers:
- Name: SimpleEcu1_LIN1
# At line 18, column 0: Element "LinControllers" is already defined in path "/"
LinControllers:
- Name: SomeOtherValue
)raw";
    std::stringstream warnings;
    bool yamlValid = ValidateWithSchema(yamlString, warnings);
    EXPECT_FALSE(yamlValid) << "YamlValidator warnings: " << warnings.str();
    std::cout << "YamlValidator warnings: " << warnings.str() << std::endl;
    EXPECT_GT(warnings.str().size(), 0u);
}

TEST_F(Test_YamlValidator, validate_repeated_container_keys_in_sequence)
{
    // Regression: the same container key (e.g. Replay) appearing in different elements of a
    // sequence must NOT be reported as a duplicate. Duplicate detection is per-map only.
    auto yamlString = R"yaml(
schemaVersion: 1
ParticipantName: P1
CanControllers:
  - Name: CAN1
    Replay:
      UseTraceSource: Source1
      MdfChannel:
        ChannelName: Ch1
  - Name: CAN2
    Replay:
      UseTraceSource: Source2
      MdfChannel:
        ChannelName: Ch2
)yaml";

    std::stringstream warnings;
    bool yamlValid = ValidateWithSchema(yamlString, warnings);
    EXPECT_TRUE(yamlValid) << "YamlValidator warnings: " << warnings.str();
    EXPECT_EQ(warnings.str(), "") << "Repeated keys across sequence elements are not duplicates";
}

TEST_F(Test_YamlValidator, validate_unnamed_children)
{
    auto yamlString = R"yaml(
ParticipantName: P1
CanControllers:
  - Name: CAN1
    UseTraceSinks:
    - Sink1
    #misplaced logger
    Logging:
      Sinks:
      - Type: Remote
Description:  foo
#correct place
Logging:
  Sinks:
  - Type: Remote
)yaml";

    std::stringstream warnings;
    bool yamlValid = ValidateWithSchema(yamlString, warnings);
    EXPECT_FALSE(yamlValid) << "YamlValidator warnings: " << warnings.str();
    std::cout << "YamlValidator warnings: " << warnings.str() << std::endl;
    EXPECT_TRUE(warnings.str().size() > 0u);
}

TEST_F(Test_YamlValidator, validate_full_participant_configuration)
{
    auto Validate = [](const std::string& path) {
        auto ReadTextFile = [](const std::string& path) -> std::string {
            std::ifstream file{path};
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        };

        auto text{ReadTextFile(path)};

        std::stringstream warningsStream;

        const bool valid{ValidateWithSchema(text, warningsStream)};
        EXPECT_TRUE(valid);

        auto warnings{warningsStream.str()};
        ASSERT_EQ(warnings, "");
    };

    Validate("ParticipantConfiguration_Full.json");
    Validate("ParticipantConfiguration_Full.yaml");
}

} // anonymous namespace

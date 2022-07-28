/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "YamlParser.hpp"
#include "YamlValidator.hpp"

#include "ParticipantConfiguration.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

class YamlValidatorTest : public testing::Test
{
};

using namespace SilKit::Config;

TEST_F(YamlValidatorTest, yaml_doc_relations)
{
    YamlValidator v;
    // Ensure that YAML validation of top-level elements works
    EXPECT_TRUE(v.IsRootElement("/SchemaVersion"));
    EXPECT_TRUE(v.IsRootElement("/ParticipantName"));
    EXPECT_TRUE(v.IsRootElement("/Description"));
    EXPECT_TRUE(v.IsRootElement("/CanControllers"));
    EXPECT_TRUE(v.IsRootElement("/LinControllers"));
    EXPECT_TRUE(v.IsRootElement("/EthernetControllers"));
    EXPECT_TRUE(v.IsRootElement("/FlexrayControllers"));
    EXPECT_TRUE(v.IsRootElement("/Logging"));
    EXPECT_TRUE(v.IsRootElement("/HealthCheck"));
    EXPECT_TRUE(v.IsRootElement("/Tracing"));
    EXPECT_TRUE(v.IsRootElement("/Extensions"));
    EXPECT_TRUE(v.IsRootElement("/Middleware"));

    EXPECT_FALSE(v.IsRootElement(" /CanControllers"));
    EXPECT_FALSE(v.IsRootElement("/Tracing/TraceSinks"));
}

TEST_F(YamlValidatorTest, validate_without_warnings)
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
    auto jsonString = yaml_to_json(to_yaml(cfg));
    YamlValidator validator;
    auto isValid = validator.Validate(jsonString, stream);
    EXPECT_TRUE(isValid);
    auto warnings = stream.str();
    EXPECT_TRUE(warnings.empty()) << "Warnings: " << warnings;
}

TEST_F(YamlValidatorTest, validate_unknown_toplevel)
{
    auto yamlString = R"yaml(
SchemaVersion: 1
ParticipantName: CanDemoParticipant
Description: Sample configuration for CAN
#typo in a toplevel statement, additional 's'
CanControllerss:
)yaml";

    std::stringstream warnings;
    YamlValidator validator;
    bool yamlValid = validator.Validate(yamlString, warnings);
    std::cout << "Yaml Validator warnings: " << warnings.str() << std::endl;
    EXPECT_TRUE(yamlValid && warnings.str().size() > 0) << "Yaml Validator warnings: " << warnings.str();
    EXPECT_TRUE(warnings.str().size() > 0);
}

TEST_F(YamlValidatorTest, validate_duplicate_element)
{
    const auto yamlString = R"raw(
LinControllers:
- Name: SimpleEcu1_LIN1
# At line 18, column 0: Element "LinControllers" is already defined in path "/"
LinControllers:
- Name: SimpleEcu1_LIN1
)raw";
    std::stringstream warnings;
    YamlValidator validator;
    bool yamlValid = validator.Validate(yamlString, warnings);
    EXPECT_FALSE(yamlValid) << "YamlValidator warnings: " << warnings.str();
    std::cout << "YamlValidator warnings: " << warnings.str() << std::endl;
    EXPECT_TRUE(warnings.str().size() > 0);
}

TEST_F(YamlValidatorTest, validate_unnamed_children)
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
    YamlValidator validator;
    bool yamlValid = validator.Validate(yamlString, warnings);
    EXPECT_FALSE(yamlValid) << "YamlValidator warnings: " << warnings.str();
    std::cout << "YamlValidator warnings: " << warnings.str() <<std::endl;
    EXPECT_TRUE(warnings.str().size() > 0);
}

} // anonymous namespace

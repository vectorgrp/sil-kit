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
    EXPECT_GT(warnings.str().size(),  0u)  << "Yaml Validator warnings: '" << warnings.str() << "'";;
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
    EXPECT_GT(warnings.str().size(),  0u);
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

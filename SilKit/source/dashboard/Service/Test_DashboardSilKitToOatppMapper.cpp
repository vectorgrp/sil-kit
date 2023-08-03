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


#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "YamlParser.hpp"

#include "SilKitToOatppMapper.hpp"


namespace SilKit {
namespace Dashboard {
class Test_DashboardSilKitToOatppMapper : public testing::Test
{
public:
    void SetUp() override
    {
    }

    static std::shared_ptr<ISilKitToOatppMapper> CreateService()
    {
        return std::make_shared<SilKitToOatppMapper>();
    }
};

TEST_F(Test_DashboardSilKitToOatppMapper, CreateSimulationCreationRequestDto_MapEndpoint)
{
    // Arrange
    const std::string endpoint("silkit://myhost:1234");
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const uint64_t expectedStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
        
    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateSimulationCreationRequestDto(endpoint, expectedStartTime);

    // Assert
    ASSERT_STREQ(dto->configuration->connectUri->c_str(), endpoint.c_str());
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateSimulationCreationRequestDto_StartTimeEqualsCreationTime)
{
    // Arrange
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const uint64_t expectedStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateSimulationCreationRequestDto("", expectedStartTime);

    // Assert
    ASSERT_EQ(dto->started, expectedStartTime);
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateSystemStatusDto_MapState)
{
    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateSystemStatusDto(Services::Orchestration::SystemState::ReadyToRun);

    // Assert
    ASSERT_EQ(dto->state, SystemState::ReadyToRun);
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateParticipantStatusDto_MapReasonAndTimeAndState)
{
    namespace orchestration = Services::Orchestration;

    // Arrange
    const std::string expectedReason("myReason");
    const auto now = std::chrono::system_clock::now();
    const auto expectedEnterTime = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    orchestration::ParticipantStatus participant_status;
    participant_status.participantName = "";
    participant_status.state = orchestration::ParticipantState::ReadyToRun;
    participant_status.enterReason = expectedReason;
    participant_status.enterTime = now;
    participant_status.refreshTime = now;

    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateParticipantStatusDto(participant_status);

    // Assert
    ASSERT_STREQ(dto->enterReason->c_str(), expectedReason.c_str());
    ASSERT_EQ(dto->enterTime, static_cast<uint64_t>(expectedEnterTime));
    ASSERT_EQ(dto->state, ParticipantState::ReadyToRun);
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateServiceDto_MapNameAndNetworkName)
{
    // Arrange
    const std::string expectedName("myService");
    const std::string expectedNetwork("myNetwork");
    Core::ServiceDescriptor descriptor;
    descriptor.SetServiceName(expectedName);
    descriptor.SetNetworkName(expectedNetwork);

    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateServiceDto(descriptor);

    // Assert
    ASSERT_STREQ(dto->name->c_str(), expectedName.c_str());
    ASSERT_STREQ(dto->networkName->c_str(), expectedNetwork.c_str());
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateDataPublisherDto_MapNameAndNetworkNameAndTopicAndMediaTypeAndLabel)
{
    // Arrange
    Core::ServiceDescriptor descriptor;
    const std::string expectedName("myService");
    const std::string expectedNetwork("myNetwork");
    descriptor.SetServiceName(expectedName);
    descriptor.SetNetworkName(expectedNetwork);

    const std::string expectedTopic("myTopic");
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataPublisherTopic, expectedTopic);

    const std::string expectedMediaType("myMediaType");
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataPublisherMediaType, expectedMediaType);

    Services::MatchingLabel expectedLabel;
    expectedLabel.key = "myKey";
    expectedLabel.value = "myValue";
    expectedLabel.kind = Services::MatchingLabel::Kind::Mandatory;
    auto labels = std::vector<Services::MatchingLabel>{expectedLabel};
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataPublisherPubLabels,
                                       Config::Serialize(labels));

    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateDataPublisherDto(descriptor);

    // Assert
    ASSERT_STREQ(dto->name->c_str(), expectedName.c_str());
    ASSERT_STREQ(dto->networkName->c_str(), expectedNetwork.c_str());
    ASSERT_STREQ(dto->spec->topic->c_str(), expectedTopic.c_str());
    ASSERT_STREQ(dto->spec->mediaType->c_str(), expectedMediaType.c_str());
    ASSERT_STREQ(dto->spec->labels->at(0)->key->c_str(), expectedLabel.key.c_str());
    ASSERT_STREQ(dto->spec->labels->at(0)->value->c_str(), expectedLabel.value.c_str());
    ASSERT_EQ(dto->spec->labels->at(0)->kind, LabelKind::Mandatory);
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateDataSubscriberDto_MapNetworkNameAndTopicAndMediaTypeAndLabel)
{
    // Arrange
    Core::ServiceDescriptor descriptor;
    const std::string expectedName("myService");
    descriptor.SetServiceName(expectedName);

    const std::string expectedTopic("myTopic");
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataSubscriberTopic, expectedTopic);

    const std::string expectedMediaType("myMediaType");
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataSubscriberMediaType, expectedMediaType);

    Services::MatchingLabel expectedLabel;
    expectedLabel.key = "myKey";
    expectedLabel.value = "myValue";
    expectedLabel.kind = Services::MatchingLabel::Kind::Mandatory;
    auto labels = std::vector<Services::MatchingLabel>{expectedLabel};
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataSubscriberSubLabels, Config::Serialize(labels));

    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateDataSubscriberDto(descriptor);

    // Assert
    ASSERT_STREQ(dto->name->c_str(), expectedName.c_str());
    ASSERT_STREQ(dto->spec->topic->c_str(), expectedTopic.c_str());
    ASSERT_STREQ(dto->spec->mediaType->c_str(), expectedMediaType.c_str());
    ASSERT_STREQ(dto->spec->labels->at(0)->key->c_str(), expectedLabel.key.c_str());
    ASSERT_STREQ(dto->spec->labels->at(0)->value->c_str(), expectedLabel.value.c_str());
    ASSERT_EQ(dto->spec->labels->at(0)->kind, LabelKind::Mandatory);
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateRpcClientDto_MapNetworkNameAndFunctionNameAndMediaTypeAndLabel)
{
    // Arrange 
    Core::ServiceDescriptor descriptor;
    const std::string expectedName("myService");
    const std::string expectedNetwork("myNetwork");
    descriptor.SetServiceName(expectedName);
    descriptor.SetNetworkName(expectedNetwork);

    const std::string expectedFunctionName("myFunctionName");
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyRpcClientFunctionName, expectedFunctionName);

    const std::string expectedMediaType("myMediaType");
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyRpcClientMediaType, expectedMediaType);

    Services::MatchingLabel expectedLabel;
    expectedLabel.key = "myKey";
    expectedLabel.value = "myValue";
    expectedLabel.kind = Services::MatchingLabel::Kind::Mandatory;
    auto labels = std::vector<Services::MatchingLabel>{expectedLabel};
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyRpcClientLabels,
                                       Config::Serialize(labels));

    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateRpcClientDto(descriptor);

    // Assert
    ASSERT_STREQ(dto->name->c_str(), expectedName.c_str());
    ASSERT_STREQ(dto->networkName->c_str(), expectedNetwork.c_str());
    ASSERT_STREQ(dto->spec->functionName->c_str(), expectedFunctionName.c_str());
    ASSERT_STREQ(dto->spec->mediaType->c_str(), expectedMediaType.c_str());
    ASSERT_STREQ(dto->spec->labels->at(0)->key->c_str(), expectedLabel.key.c_str());
    ASSERT_STREQ(dto->spec->labels->at(0)->value->c_str(), expectedLabel.value.c_str());
    ASSERT_EQ(dto->spec->labels->at(0)->kind, LabelKind::Mandatory);
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateRpcServerDto_MapNetworkNameAndFunctionNameAndMediaTypeAndLabel)
{
    // Arrange
    Core::ServiceDescriptor descriptor;
    const std::string expectedName("myService");
    const std::string expectedFunctionName("myFunctionName");
    descriptor.SetServiceName(expectedName);
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyRpcServerFunctionName, expectedFunctionName);

    const std::string expectedMediaType("myMediaType");
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyRpcServerMediaType, expectedMediaType);

    Services::MatchingLabel expectedLabel;
    expectedLabel.key = "myKey";
    expectedLabel.value = "myValue";
    expectedLabel.kind = Services::MatchingLabel::Kind::Mandatory;
    auto labels = std::vector<Services::MatchingLabel>{expectedLabel};
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyRpcServerLabels, Config::Serialize(labels));

    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateRpcServerDto(descriptor);

    // Assert
    ASSERT_STREQ(dto->name->c_str(), expectedName.c_str());
    ASSERT_STREQ(dto->spec->functionName->c_str(), expectedFunctionName.c_str());
    ASSERT_STREQ(dto->spec->mediaType->c_str(), expectedMediaType.c_str());
    ASSERT_STREQ(dto->spec->labels->at(0)->key->c_str(), expectedLabel.key.c_str());
    ASSERT_STREQ(dto->spec->labels->at(0)->value->c_str(), expectedLabel.value.c_str());
    ASSERT_EQ(dto->spec->labels->at(0)->kind, LabelKind::Mandatory);
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateSimulationEndDto)
{
    // Arrange 
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const uint64_t expectedStopTime = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateSimulationEndDto(expectedStopTime);

    // Assert
    ASSERT_EQ(dto->stopped, expectedStopTime);
}

} // namespace Dashboard
} // namespace SilKit

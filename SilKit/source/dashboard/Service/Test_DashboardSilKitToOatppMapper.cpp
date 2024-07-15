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
#include "fmt/core.h"

#include <algorithm>


namespace SilKit {
namespace Dashboard {
class Test_DashboardSilKitToOatppMapper : public testing::Test
{
public:
    void SetUp() override {}

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
    const auto expectedEnterTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

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
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataPublisherPubLabels, Config::Serialize(labels));

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
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyRpcClientLabels, Config::Serialize(labels));

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

TEST_F(Test_DashboardSilKitToOatppMapper, CreateBulkControllerDto)
{
    // Arrange
    constexpr SilKit::Core::EndpointId expectedId{12345};
    const std::string expectedName("myService");
    const std::string expectedNetwork("myNetwork");
    Core::ServiceDescriptor descriptor;
    descriptor.SetServiceId(expectedId);
    descriptor.SetServiceName(expectedName);
    descriptor.SetNetworkName(expectedNetwork);

    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateBulkControllerDto(descriptor);

    // Assert
    ASSERT_EQ(dto->id.getValue(0), expectedId);
    ASSERT_STREQ(dto->name->c_str(), expectedName.c_str());
    ASSERT_STREQ(dto->networkName->c_str(), expectedNetwork.c_str());
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateBulkDataServiceDto_MapNetworkNameAndTopicAndMediaTypeAndLabel)
{
    // Arrange
    Core::ServiceDescriptor descriptor;

    constexpr SilKit::Core::EndpointId expectedId{12345};
    descriptor.SetServiceId(expectedId);

    const std::string expectedNetwork("myNetwork");
    descriptor.SetNetworkName(expectedNetwork);

    const std::string expectedName("myService");
    descriptor.SetServiceName(expectedName);

    descriptor.SetSupplementalDataItem(SilKit::Core::Discovery::controllerType,
                                       SilKit::Core::Discovery::controllerTypeDataSubscriber);

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
    const auto dto = dataMapper->CreateBulkDataServiceDto(descriptor);

    // Assert
    ASSERT_EQ(dto->id.getValue(0), expectedId);
    ASSERT_STREQ(dto->name->c_str(), expectedName.c_str());
    ASSERT_STREQ(dto->networkName->c_str(), expectedNetwork.c_str());
    ASSERT_STREQ(dto->spec->topic->c_str(), expectedTopic.c_str());
    ASSERT_STREQ(dto->spec->mediaType->c_str(), expectedMediaType.c_str());
    ASSERT_STREQ(dto->spec->labels->at(0)->key->c_str(), expectedLabel.key.c_str());
    ASSERT_STREQ(dto->spec->labels->at(0)->value->c_str(), expectedLabel.value.c_str());
    ASSERT_EQ(dto->spec->labels->at(0)->kind, LabelKind::Mandatory);
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateBulkRpcServiceDto_MapNetworkNameAndFunctionNameAndMediaTypeAndLabel)
{
    // Arrange
    Core::ServiceDescriptor descriptor;
    const SilKit::Core::EndpointId expectedId{12345};
    const std::string expectedName("myService");
    const std::string expectedNetwork("myNetwork");
    descriptor.SetServiceId(expectedId);
    descriptor.SetServiceName(expectedName);
    descriptor.SetNetworkName(expectedNetwork);

    descriptor.SetSupplementalDataItem(SilKit::Core::Discovery::controllerType,
                                       SilKit::Core::Discovery::controllerTypeRpcClient);

    const std::string expectedFunctionName("myFunctionName");
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyRpcClientFunctionName, expectedFunctionName);

    const std::string expectedMediaType("myMediaType");
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyRpcClientMediaType, expectedMediaType);

    Services::MatchingLabel expectedLabel;
    expectedLabel.key = "myKey";
    expectedLabel.value = "myValue";
    expectedLabel.kind = Services::MatchingLabel::Kind::Mandatory;
    auto labels = std::vector<Services::MatchingLabel>{expectedLabel};
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyRpcClientLabels, Config::Serialize(labels));

    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateBulkRpcServiceDto(descriptor);

    // Assert
    ASSERT_EQ(dto->id.getValue(0), expectedId);
    ASSERT_STREQ(dto->name->c_str(), expectedName.c_str());
    ASSERT_STREQ(dto->networkName->c_str(), expectedNetwork.c_str());
    ASSERT_STREQ(dto->spec->functionName->c_str(), expectedFunctionName.c_str());
    ASSERT_STREQ(dto->spec->mediaType->c_str(), expectedMediaType.c_str());
    ASSERT_STREQ(dto->spec->labels->at(0)->key->c_str(), expectedLabel.key.c_str());
    ASSERT_STREQ(dto->spec->labels->at(0)->value->c_str(), expectedLabel.value.c_str());
    ASSERT_EQ(dto->spec->labels->at(0)->kind, LabelKind::Mandatory);
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateBulkServiceInternalDto_RpcServerInternal)
{
    // Arrange
    Core::ServiceDescriptor descriptor;
    const SilKit::Core::EndpointId expectedId{12345};
    const std::string expectedName("myService");
    const std::string expectedNetwork("myNetwork");
    descriptor.SetServiceId(expectedId);
    descriptor.SetServiceName(expectedName);
    descriptor.SetNetworkName(expectedNetwork);

    const SilKit::Core::EndpointId expectedParentId{54321};
    descriptor.SetSupplementalDataItem(SilKit::Core::Discovery::supplKeyRpcServerInternalParentServiceID,
                                       std::to_string(expectedParentId));

    descriptor.SetSupplementalDataItem(SilKit::Core::Discovery::controllerType,
                                       SilKit::Core::Discovery::controllerTypeRpcServerInternal);

    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateBulkServiceInternalDto(descriptor);

    // Assert
    ASSERT_EQ(dto->id.getValue(0), expectedId);
    ASSERT_STREQ(dto->name->c_str(), expectedName.c_str());
    ASSERT_STREQ(dto->networkName->c_str(), expectedNetwork.c_str());
    ASSERT_EQ(dto->parentId.getValue(0), expectedParentId);
}

TEST_F(Test_DashboardSilKitToOatppMapper, CreateBulkSimulationDto)
{
    const auto to_ms = [](const auto tp) -> std::uint64_t {
        return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
    };

    const auto makeControllerCreatedData = [](const std::string& participantName, const std::string& serviceName,
                                              SilKit::Core::EndpointId serviceId, const std::string& networkName,
                                              const std::string& controllerType) -> SilKit::Dashboard::ServiceData {
        SilKit::Dashboard::ServiceData serviceData;
        serviceData.discoveryType = Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated;
        serviceData.serviceDescriptor.SetParticipantNameAndComputeId(participantName);
        serviceData.serviceDescriptor.SetServiceType(Core::ServiceType::Controller);
        serviceData.serviceDescriptor.SetServiceName(serviceName);
        serviceData.serviceDescriptor.SetServiceId(serviceId);
        serviceData.serviceDescriptor.SetNetworkName(networkName);
        serviceData.serviceDescriptor.SetSupplementalDataItem(SilKit::Core::Discovery::controllerType, controllerType);
        return serviceData;
    };

    const auto makeLinkCreatedData = [](const std::string& participantName, SilKit::Config::NetworkType networkType,
                                        const std::string& networkName) -> SilKit::Dashboard::ServiceData {
        SilKit::Dashboard::ServiceData serviceData;
        serviceData.discoveryType = Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated;
        serviceData.serviceDescriptor.SetParticipantNameAndComputeId(participantName);
        serviceData.serviceDescriptor.SetServiceType(Core::ServiceType::Link);
        serviceData.serviceDescriptor.SetNetworkType(networkType);
        serviceData.serviceDescriptor.SetNetworkName(networkName);
        return serviceData;
    };

    // Arrange
    DashboardBulkUpdate expectedBulkUpdate;

    expectedBulkUpdate.stopped = std::make_unique<std::uint64_t>(12345u);

    const auto expectedSystemState0 = SilKit::Dashboard::SystemState::Shutdown;
    expectedBulkUpdate.systemStates.emplace_back(SilKit::Services::Orchestration::SystemState::Shutdown);

    const auto expectedSystemState1 = SilKit::Dashboard::SystemState::Running;
    expectedBulkUpdate.systemStates.emplace_back(SilKit::Services::Orchestration::SystemState::Running);

    const auto expectedSystemState2 = SilKit::Dashboard::SystemState::Paused;
    expectedBulkUpdate.systemStates.emplace_back(SilKit::Services::Orchestration::SystemState::Paused);

    constexpr auto expectedAParticipantState0 = SilKit::Dashboard::ParticipantState::Running;
    const SilKit::Services::Orchestration::ParticipantStatus aParticipantStatus0{
        "A", SilKit::Services::Orchestration::ParticipantState::Running, "A Reason 1",
        std::chrono::system_clock::now() + std::chrono::seconds{1000},
        std::chrono::system_clock::now() + std::chrono::seconds{2000}};

    constexpr auto expectedAParticipantState1 = SilKit::Dashboard::ParticipantState::Shutdown;
    const SilKit::Services::Orchestration::ParticipantStatus aParticipantStatus1{
        "A", SilKit::Services::Orchestration::ParticipantState::Shutdown, "A Reason 2",
        std::chrono::system_clock::now() + std::chrono::seconds{3000},
        std::chrono::system_clock::now() + std::chrono::seconds{4000}};

    constexpr auto expectedBParticipantState = SilKit::Dashboard::ParticipantState::Stopped;
    const SilKit::Services::Orchestration::ParticipantStatus bParticipantStatus{
        "B", SilKit::Services::Orchestration::ParticipantState::Stopped, "B Reason",
        std::chrono::system_clock::now() + std::chrono::seconds{5000},
        std::chrono::system_clock::now() + std::chrono::seconds{6000}};

    expectedBulkUpdate.participantStatuses.emplace_back(aParticipantStatus0);
    expectedBulkUpdate.participantStatuses.emplace_back(aParticipantStatus1);
    expectedBulkUpdate.participantStatuses.emplace_back(bParticipantStatus);

    expectedBulkUpdate.participantConnectionInformations.emplace_back(
        SilKit::Services::Orchestration::ParticipantConnectionInformation{"C"});

    expectedBulkUpdate.serviceDatas.emplace_back(
        makeControllerCreatedData("A", "ACan0", 1001, "ACanNet0", SilKit::Core::Discovery::controllerTypeCan));
    expectedBulkUpdate.serviceDatas.emplace_back(
        makeControllerCreatedData("A", "ACan1", 1002, "ACanNet1", SilKit::Core::Discovery::controllerTypeCan));

    expectedBulkUpdate.serviceDatas.emplace_back(
        makeControllerCreatedData("A", "AEth0", 2001, "AEthNet0", SilKit::Core::Discovery::controllerTypeEthernet));
    expectedBulkUpdate.serviceDatas.emplace_back(
        makeControllerCreatedData("A", "AEth1", 2002, "AEthNet1", SilKit::Core::Discovery::controllerTypeEthernet));

    expectedBulkUpdate.serviceDatas.emplace_back(
        makeControllerCreatedData("A", "AFlr0", 2001, "AFlrNet0", SilKit::Core::Discovery::controllerTypeFlexray));
    expectedBulkUpdate.serviceDatas.emplace_back(
        makeControllerCreatedData("A", "AFlr1", 2002, "AFlrNet1", SilKit::Core::Discovery::controllerTypeFlexray));

    expectedBulkUpdate.serviceDatas.emplace_back(
        makeControllerCreatedData("A", "ALin0", 2001, "ALinNet0", SilKit::Core::Discovery::controllerTypeLin));
    expectedBulkUpdate.serviceDatas.emplace_back(
        makeControllerCreatedData("A", "ALin1", 2002, "ALinNet1", SilKit::Core::Discovery::controllerTypeLin));

    expectedBulkUpdate.serviceDatas.emplace_back(makeLinkCreatedData("A", Config::NetworkType::CAN, "ACanNet0"));
    expectedBulkUpdate.serviceDatas.emplace_back(makeLinkCreatedData("A", Config::NetworkType::CAN, "ACanNet1"));

    expectedBulkUpdate.serviceDatas.emplace_back(makeLinkCreatedData("A", Config::NetworkType::Ethernet, "AEthNet0"));
    expectedBulkUpdate.serviceDatas.emplace_back(makeLinkCreatedData("A", Config::NetworkType::Ethernet, "AEthNet1"));

    expectedBulkUpdate.serviceDatas.emplace_back(makeLinkCreatedData("A", Config::NetworkType::FlexRay, "AFlrNet0"));
    expectedBulkUpdate.serviceDatas.emplace_back(makeLinkCreatedData("A", Config::NetworkType::FlexRay, "AFlrNet1"));

    expectedBulkUpdate.serviceDatas.emplace_back(makeLinkCreatedData("A", Config::NetworkType::LIN, "ALinNet0"));
    expectedBulkUpdate.serviceDatas.emplace_back(makeLinkCreatedData("A", Config::NetworkType::LIN, "ALinNet1"));

    // Act
    const auto dataMapper = CreateService();
    const auto dto = dataMapper->CreateBulkSimulationDto(expectedBulkUpdate);

    // Assert
    ASSERT_NE(dto->stopped.getPtr(), nullptr);
    ASSERT_EQ(dto->stopped.getValue(0), *expectedBulkUpdate.stopped);

    ASSERT_NE(dto->system.getPtr(), nullptr);
    ASSERT_NE(dto->system->statuses.getPtr(), nullptr);
    ASSERT_EQ(dto->system->statuses->size(), expectedBulkUpdate.systemStates.size());
    ASSERT_NE(dto->system->statuses[0].getPtr(), nullptr);
    ASSERT_NE(dto->system->statuses[0]->state.getPtr(), nullptr);
    ASSERT_EQ(dto->system->statuses[0]->state, expectedSystemState0);
    ASSERT_NE(dto->system->statuses[1].getPtr(), nullptr);
    ASSERT_NE(dto->system->statuses[1]->state.getPtr(), nullptr);
    ASSERT_EQ(dto->system->statuses[1]->state, expectedSystemState1);
    ASSERT_NE(dto->system->statuses[2].getPtr(), nullptr);
    ASSERT_NE(dto->system->statuses[2]->state.getPtr(), nullptr);
    ASSERT_EQ(dto->system->statuses[2]->state, expectedSystemState2);

    ASSERT_NE(dto->participants.getPtr(), nullptr);
    ASSERT_EQ(dto->participants->size(), 3);

    oatpp::Object<BulkParticipantDto> aParticipantDto;
    oatpp::Object<BulkParticipantDto> bParticipantDto;
    oatpp::Object<BulkParticipantDto> cParticipantDto;

    for (const auto& participantDto : *dto->participants.get())
    {
        ASSERT_NE(participantDto.getPtr(), nullptr);
        ASSERT_NE(participantDto->name.getPtr(), nullptr);

        if (participantDto->name == "A")
        {
            aParticipantDto = participantDto;
        }

        if (participantDto->name == "B")
        {
            bParticipantDto = participantDto;
        }

        if (participantDto->name == "C")
        {
            cParticipantDto = participantDto;
        }
    }

    ASSERT_NE(aParticipantDto.getPtr(), nullptr);
    ASSERT_NE(aParticipantDto->statuses.getPtr(), nullptr);
    ASSERT_EQ(aParticipantDto->statuses->size(), 2);
    ASSERT_NE(aParticipantDto->statuses[0].getPtr(), nullptr);
    ASSERT_EQ(aParticipantDto->statuses[0]->state, expectedAParticipantState0);
    ASSERT_EQ(aParticipantDto->statuses[0]->enterReason, aParticipantStatus0.enterReason);
    ASSERT_EQ(aParticipantDto->statuses[0]->enterTime, to_ms(aParticipantStatus0.enterTime));
    ASSERT_NE(aParticipantDto->statuses[1].getPtr(), nullptr);
    ASSERT_EQ(aParticipantDto->statuses[1]->state, expectedAParticipantState1);
    ASSERT_EQ(aParticipantDto->statuses[1]->enterReason, aParticipantStatus1.enterReason);
    ASSERT_EQ(aParticipantDto->statuses[1]->enterTime, to_ms(aParticipantStatus1.enterTime));

    ASSERT_NE(aParticipantDto->canControllers.getPtr(), nullptr);
    ASSERT_EQ(aParticipantDto->canControllers->size(), 2);
    ASSERT_NE(aParticipantDto->canControllers[0].getPtr(), nullptr);
    ASSERT_NE(aParticipantDto->canControllers[1].getPtr(), nullptr);

    ASSERT_NE(aParticipantDto->ethernetControllers.getPtr(), nullptr);
    ASSERT_EQ(aParticipantDto->ethernetControllers->size(), 2);
    ASSERT_NE(aParticipantDto->ethernetControllers[0].getPtr(), nullptr);
    ASSERT_NE(aParticipantDto->ethernetControllers[1].getPtr(), nullptr);

    ASSERT_NE(aParticipantDto->flexrayControllers.getPtr(), nullptr);
    ASSERT_EQ(aParticipantDto->flexrayControllers->size(), 2);
    ASSERT_NE(aParticipantDto->flexrayControllers[0].getPtr(), nullptr);
    ASSERT_NE(aParticipantDto->flexrayControllers[1].getPtr(), nullptr);

    ASSERT_NE(aParticipantDto->linControllers.getPtr(), nullptr);
    ASSERT_EQ(aParticipantDto->linControllers->size(), 2);
    ASSERT_NE(aParticipantDto->linControllers[0].getPtr(), nullptr);
    ASSERT_NE(aParticipantDto->linControllers[1].getPtr(), nullptr);

    ASSERT_NE(aParticipantDto->canNetworks.getPtr(), nullptr);
    ASSERT_EQ(aParticipantDto->canNetworks->size(), 2);

    ASSERT_NE(aParticipantDto->ethernetNetworks.getPtr(), nullptr);
    ASSERT_EQ(aParticipantDto->ethernetNetworks->size(), 2);

    ASSERT_NE(aParticipantDto->flexrayNetworks.getPtr(), nullptr);
    ASSERT_EQ(aParticipantDto->flexrayNetworks->size(), 2);

    ASSERT_NE(aParticipantDto->linNetworks.getPtr(), nullptr);
    ASSERT_EQ(aParticipantDto->linNetworks->size(), 2);

    ASSERT_NE(bParticipantDto.getPtr(), nullptr);
    ASSERT_NE(bParticipantDto->statuses.getPtr(), nullptr);
    ASSERT_EQ(bParticipantDto->statuses->size(), 1);
    ASSERT_NE(bParticipantDto->statuses[0].getPtr(), nullptr);
    ASSERT_EQ(bParticipantDto->statuses[0]->state, expectedBParticipantState);
    ASSERT_EQ(bParticipantDto->statuses[0]->enterReason, bParticipantStatus.enterReason);
    ASSERT_EQ(bParticipantDto->statuses[0]->enterTime, to_ms(bParticipantStatus.enterTime));

    ASSERT_NE(cParticipantDto.getPtr(), nullptr);
}

} // namespace Dashboard
} // namespace SilKit

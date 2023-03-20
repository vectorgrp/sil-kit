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

#include "MockParticipant.hpp"

#include "SilKitEventHandler.hpp"

#include "Mocks/MockSilKitToOatppMapper.hpp"
#include "Mocks/MockDashboardSystemServiceClient.hpp"

using namespace testing;

namespace SilKit {
namespace Dashboard {

class TestDashboardSilKitEventHandler : public Test
{
public:
    void SetUp() override
    {
        _mockDashboardSystemServiceClient = std::make_shared<StrictMock<MockDashboardSystemServiceClient>>();
        _mockSilKitToOatppMapper = std::make_shared<StrictMock<MockSilKitToOatppMapper>>();

        EXPECT_CALL(_dummyLogger, GetLogLevel).WillRepeatedly(Return(Services::Logging::Level::Info));
    }

    std::shared_ptr<SilKitEventHandler> CreateService()
    {
        return std::make_shared<SilKitEventHandler>(&_dummyLogger, _mockDashboardSystemServiceClient,
                                                    _mockSilKitToOatppMapper);
    }

    std::shared_ptr<SilKitEventHandler> CreateService(oatpp::UInt64 expectedSimulationId)
    {
        const auto service = CreateService();
        service->_simulationId = expectedSimulationId;
        return service;
    }

    Core::Tests::MockLogger _dummyLogger;
    std::shared_ptr<StrictMock<MockDashboardSystemServiceClient>> _mockDashboardSystemServiceClient;
    std::shared_ptr<StrictMock<MockSilKitToOatppMapper>> _mockSilKitToOatppMapper;
};

TEST_F(TestDashboardSilKitEventHandler, OnStart_CreateSimulationSent_NoResponse)
{
    // Arrange
    auto request = SimulationCreationRequestDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateSimulationCreationRequestDto).WillOnce(Return(request));
    std::promise<oatpp::Object<SimulationCreationResponseDto>> simulationCreationPromise;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, CreateSimulation(request)).WillOnce(Return(simulationCreationPromise.get_future()));
    EXPECT_CALL(_dummyLogger, Info("Dashboard: creating simulation"))
        .WillOnce(Return());
    EXPECT_CALL(_dummyLogger, Warn("Dashboard: creating simulation: giving up...")).WillOnce(Return());

    // Act
    std::future<bool> res;
    {
        const auto service = CreateService();
        res = service->OnStart("silkit://localhost:8500", 0);
    }

    // Assert
    ASSERT_FALSE(res.get());
}

TEST_F(TestDashboardSilKitEventHandler, OnStart_CreateSimulationSent_Success)
{
    // Arrange
    const oatpp::UInt64 expectedSimulationId = 123;
    auto request = SimulationCreationRequestDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateSimulationCreationRequestDto).WillOnce(Return(request));
    std::promise<oatpp::Object<SimulationCreationResponseDto>> simulationCreationPromise;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, CreateSimulation(request))
        .WillOnce(Return(simulationCreationPromise.get_future()));
    EXPECT_CALL(_dummyLogger, Info("Dashboard: creating simulation")).WillOnce(Return());
    EXPECT_CALL(_dummyLogger, Log(Services::Logging::Level::Info, "Dashboard: created simulation with id 123"))
        .WillOnce(Return());
    const auto service = CreateService();

    // Act
    auto res = service->OnStart("silkit://localhost:8500", 0);
    auto response = SimulationCreationResponseDto::createShared();
    response->id = expectedSimulationId;
    simulationCreationPromise.set_value(response);

    // Assert
    ASSERT_TRUE(res.get());
}

TEST_F(TestDashboardSilKitEventHandler, OnStart_CreateSimulatioSend_Failed)
{
    // Arrange
    const oatpp::UInt64 expectedSimulationId = 123;
    auto request = SimulationCreationRequestDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateSimulationCreationRequestDto).WillOnce(Return(request));
    std::promise<oatpp::Object<SimulationCreationResponseDto>> simulationCreationPromise;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, CreateSimulation(request))
        .WillOnce(Return(simulationCreationPromise.get_future()));
    EXPECT_CALL(_dummyLogger, Info("Dashboard: creating simulation")).WillOnce(Return());
    const auto service = CreateService();

    // Act
    auto res = service->OnStart("silkit://localhost:8500", 0);
    simulationCreationPromise.set_value(nullptr);

    // Assert
    ASSERT_FALSE(res.get());
}

TEST_F(TestDashboardSilKitEventHandler, OnShutdown_SimulationEndRequestSent)
{
    // Arrange
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);
    EXPECT_CALL(_dummyLogger, Log(Services::Logging::Level::Info, "Dashboard: setting end for simulation 123"))
        .WillOnce(Return());

    // Setup request for OnShutdown()
    auto request = SimulationEndDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateSimulationEndDto).WillOnce(Return(request));
    EXPECT_CALL(*_mockDashboardSystemServiceClient, SetSimulationEnd(expectedSimulationId, request)).Times(1);
    
    // Act
    service->OnShutdown(0);
}

TEST_F(TestDashboardSilKitEventHandler, OnParticipantConnected_AddParticipantRequestIsSent)
{
    using connectionInfo = Services::Orchestration::ParticipantConnectionInformation;

    // Arrange
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualParticipantName;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddParticipantToSimulation)
        .WillOnce(DoAll(WithArgs<0, 1>([&](auto simulationId, auto participantName) {
            actualSimulationId = simulationId;
            actualParticipantName = participantName;
        })));

    // Act
    const connectionInfo info{"my Participant"};
    service->OnParticipantConnected(info);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualParticipantName->c_str(), "my%20Participant");
}

TEST_F(TestDashboardSilKitEventHandler, OnSystemStateChanged_UpdateSystemStatusRequestSent)
{
    using systemState = Services::Orchestration::SystemState;

    // Arrange
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    auto dummyRequestDto = SystemStatusDto::createShared();
    auto expectedSystemState = SilKit::Dashboard::SystemState::Running;
    dummyRequestDto->state = expectedSystemState;
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateSystemStatusDto).WillOnce(Return(dummyRequestDto));
    oatpp::UInt64 actualSimulationId;
    SilKit::Dashboard::SystemState actualSystemState;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, UpdateSystemStatusForSimulation)
        .WillOnce(DoAll(WithArgs<0, 1>([&](auto simulationId, auto systemStatus) {
            actualSimulationId = simulationId;
            actualSystemState = systemStatus->state;
        })));

    // Act
    service->OnSystemStateChanged(systemState::Running);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_EQ(actualSystemState, expectedSystemState);
}

TEST_F(TestDashboardSilKitEventHandler, OnParticipantStatusChanged_AddParticipantStatusRequestSent)
{
    using participantStatus = Services::Orchestration::ParticipantStatus;

    // Arrange
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    auto dummyRequestDto = ParticipantStatusDto::createShared();
    auto expectedParticipantState = SilKit::Dashboard::ParticipantState::Running;
    dummyRequestDto->state = expectedParticipantState;
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateParticipantStatusDto).WillOnce(Return(dummyRequestDto));

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualParticipantName;
    SilKit::Dashboard::ParticipantState actualParticipantState;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddParticipantStatusForSimulation)
        .WillOnce(DoAll(WithArgs<0, 1, 2>([&](auto simulationId, auto participantName, auto participantStatus) {
            actualSimulationId = simulationId;
            actualParticipantName = participantName;
            actualParticipantState = participantStatus->state;
        })));

    // Act
    participantStatus status;
    status.participantName = "my/Participant";
    service->OnParticipantStatusChanged(status);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualParticipantName->c_str(), "my%2fParticipant");
    ASSERT_EQ(actualParticipantState, expectedParticipantState);
}

Core::ServiceDescriptor BuildDescriptor(const std::string& type, SilKit::Core::EndpointId serviceId, const std::string& participant)
{
    Core::ServiceDescriptor descriptor;
    descriptor.SetServiceType(Core::ServiceType::Controller);
    descriptor.SetServiceId(serviceId);
    descriptor.SetParticipantName(participant);
    descriptor.SetSupplementalDataItem(Core::Discovery::controllerType, type);
    return descriptor;
}

TEST_F(TestDashboardSilKitEventHandler, OnServiceDiscoveryEvent_CanControllerCreated_AddCanControllerRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    SilKit::Core::EndpointId expectedServiceId = 456;
    auto descriptor = BuildDescriptor(Core::Discovery::controllerTypeCan, expectedServiceId, "my participant");
    auto dummyRequestDto = ServiceDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateServiceDto(descriptor)).WillOnce(Return(dummyRequestDto));

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualParticipantName;
    oatpp::UInt64 actualServiceId;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddCanControllerForParticipantOfSimulation)
        .WillOnce(DoAll(WithArgs<0, 1, 2>([&](auto simulationId, auto participantName, auto serviceId) {
            actualSimulationId = simulationId;
            actualParticipantName = participantName;
            actualServiceId = serviceId;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualParticipantName->c_str(), "my%20participant");
    ASSERT_EQ(actualServiceId, expectedServiceId);
}

TEST_F(TestDashboardSilKitEventHandler,
       OnServiceDiscoveryEvent_EthernetControllerCreated_AddEthernetControllerRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    SilKit::Core::EndpointId expectedServiceId = 456;
    auto descriptor = BuildDescriptor(Core::Discovery::controllerTypeEthernet, expectedServiceId, "my participant");
    auto dummyRequestDto = ServiceDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateServiceDto(descriptor)).WillOnce(Return(dummyRequestDto));

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualParticipantName;
    oatpp::UInt64 actualServiceId;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddEthernetControllerForParticipantOfSimulation)
        .WillOnce(DoAll(WithArgs<0, 1, 2>([&](auto simulationId, auto participantName, auto serviceId) {
            actualSimulationId = simulationId;
            actualParticipantName = participantName;
            actualServiceId = serviceId;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualParticipantName->c_str(), "my%20participant");
    ASSERT_EQ(actualServiceId, expectedServiceId);
}

TEST_F(TestDashboardSilKitEventHandler,
       OnServiceDiscoveryEvent_FlexrayControllerCreated_AddFlexrayControllerRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    SilKit::Core::EndpointId expectedServiceId = 456;
    auto descriptor = BuildDescriptor(Core::Discovery::controllerTypeFlexray, expectedServiceId, "my participant");
    auto dummyRequestDto = ServiceDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateServiceDto(descriptor)).WillOnce(Return(dummyRequestDto));

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualParticipantName;
    oatpp::UInt64 actualServiceId;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddFlexrayControllerForParticipantOfSimulation)
        .WillOnce(DoAll(WithArgs<0, 1, 2>([&](auto simulationId, auto participantName, auto serviceId) {
            actualSimulationId = simulationId;
            actualParticipantName = participantName;
            actualServiceId = serviceId;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualParticipantName->c_str(), "my%20participant");
    ASSERT_EQ(actualServiceId, expectedServiceId);
}

TEST_F(TestDashboardSilKitEventHandler, OnServiceDiscoveryEvent_LinControllerCreated_AddLinControllerRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    SilKit::Core::EndpointId expectedServiceId = 456;
    auto descriptor = BuildDescriptor(Core::Discovery::controllerTypeLin, expectedServiceId, "my participant");
    auto dummyRequestDto = ServiceDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateServiceDto(descriptor)).WillOnce(Return(dummyRequestDto));

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualParticipantName;
    oatpp::UInt64 actualServiceId;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddLinControllerForParticipantOfSimulation)
        .WillOnce(DoAll(WithArgs<0, 1, 2>([&](auto simulationId, auto participantName, auto serviceId) {
            actualSimulationId = simulationId;
            actualParticipantName = participantName;
            actualServiceId = serviceId;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualParticipantName->c_str(), "my%20participant");
    ASSERT_EQ(actualServiceId, expectedServiceId);
}

TEST_F(TestDashboardSilKitEventHandler, OnServiceDiscoveryEvent_DataPublisherCreated_AddDataPublisherRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    const char* expectedParticipantName{"my%20participant"};
    SilKit::Core::EndpointId expectedServiceId = 456;
    auto descriptor = BuildDescriptor(Core::Discovery::controllerTypeDataPublisher, expectedServiceId, "my participant");
    auto dummyRequestDto = DataPublisherDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateDataPublisherDto(descriptor)).WillOnce(Return(dummyRequestDto));

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualParticipantName;
    oatpp::UInt64 actualServiceId;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddDataPublisherForParticipantOfSimulation)
        .WillOnce(DoAll(WithArgs<0, 1, 2>([&](auto simulationId, auto participantName, auto serviceId) {
            actualSimulationId = simulationId;
            actualParticipantName = participantName;
            actualServiceId = serviceId;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualParticipantName->c_str(), expectedParticipantName);
    ASSERT_EQ(actualServiceId, expectedServiceId);
}

TEST_F(TestDashboardSilKitEventHandler,
       OnServiceDiscoveryEvent_DataSubscriberCreated_AddDataSubscriberRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    const char* expectedParticipantName{"my%20participant"};
    SilKit::Core::EndpointId expectedServiceId = 456;
    auto descriptor =
        BuildDescriptor(Core::Discovery::controllerTypeDataSubscriber, expectedServiceId, "my participant");
    auto dummyRequestDto = DataSubscriberDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateDataSubscriberDto(descriptor)).WillOnce(Return(dummyRequestDto));

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualParticipantName;
    oatpp::UInt64 actualServiceId;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddDataSubscriberForParticipantOfSimulation)
        .WillOnce(DoAll(WithArgs<0, 1, 2>([&](auto simulationId, auto participantName, auto serviceId) {
            actualSimulationId = simulationId;
            actualParticipantName = participantName;
            actualServiceId = serviceId;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualParticipantName->c_str(), expectedParticipantName);
    ASSERT_EQ(actualServiceId, expectedServiceId);
}

TEST_F(TestDashboardSilKitEventHandler,
       OnServiceDiscoveryEvent_DataSubscriberInternalCreated_AddDataSubscriberInternalRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const char* expectedParentServiceId{"1"};
    const char* expectedParticipantName{"my%20participant"};
    SilKit::Core::EndpointId expectedServiceId = 456;
    const auto service = CreateService(expectedSimulationId);

    auto descriptor = BuildDescriptor(Core::Discovery::controllerTypeDataSubscriberInternal, expectedServiceId, "my participant");
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataSubscriberInternalParentServiceID,
                                       expectedParentServiceId);
    auto dummyRequestDto = ServiceDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateServiceDto(descriptor)).WillOnce(Return(dummyRequestDto));

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualParticipantName;
    oatpp::String actualParentServiceId;
    oatpp::UInt64 actualServiceId;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddDataSubscriberInternalForParticipantOfSimulation)
        .WillOnce(DoAll(WithArgs<0, 1, 2, 3>([&](auto simulationId, auto participantName, auto parentServiceId, auto serviceId) {
            actualSimulationId = simulationId;
            actualParticipantName = participantName;
            actualParentServiceId = parentServiceId;
            actualServiceId = serviceId;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualParticipantName->c_str(), expectedParticipantName);
    ASSERT_STREQ(actualParentServiceId->c_str(), expectedParentServiceId);
    ASSERT_EQ(actualServiceId, expectedServiceId);
}

TEST_F(TestDashboardSilKitEventHandler,
       OnServiceDiscoveryEvent_RpcClientCreated_AddRpcClientRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    const char* expectedParticipantName{"my%20participant"};
    SilKit::Core::EndpointId expectedServiceId = 456;
    auto descriptor = BuildDescriptor(Core::Discovery::controllerTypeRpcClient, expectedServiceId, "my participant");
    auto dummyRequestDto = RpcClientDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateRpcClientDto(descriptor)).WillOnce(Return(dummyRequestDto));

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualParticipantName;
    oatpp::UInt64 actualServiceId;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddRpcClientForParticipantOfSimulation)
        .WillOnce(DoAll(WithArgs<0, 1, 2>([&](auto simulationId, auto participantName, auto serviceId) {
            actualSimulationId = simulationId;
            actualParticipantName = participantName;
            actualServiceId = serviceId;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualParticipantName->c_str(), expectedParticipantName);
    ASSERT_EQ(actualServiceId, expectedServiceId);
}

TEST_F(TestDashboardSilKitEventHandler,
       OnServiceDiscoveryEvent_RpcServerCreated_AddRpcServerRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    const char* expectedParticipantName{"my%20participant"};
    SilKit::Core::EndpointId expectedServiceId = 456;
    auto descriptor = BuildDescriptor(Core::Discovery::controllerTypeRpcServer, expectedServiceId, "my participant");
    auto dummyRequestDto = RpcServerDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateRpcServerDto(descriptor)).WillOnce(Return(dummyRequestDto));

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualParticipantName;
    oatpp::UInt64 actualServiceId;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddRpcServerForParticipantOfSimulation)
        .WillOnce(DoAll(WithArgs<0, 1, 2>([&](auto simulationId, auto participantName, auto serviceId) {
            actualSimulationId = simulationId;
            actualParticipantName = participantName;
            actualServiceId = serviceId;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualParticipantName->c_str(), expectedParticipantName);
    ASSERT_EQ(actualServiceId, expectedServiceId);
}

TEST_F(TestDashboardSilKitEventHandler,
       OnServiceDiscoveryEvent_RpcServerInternalCreated_AddRpcServerInternalRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const char* expectedParentServiceId{"1"};
    const char* expectedParticipantName{"my%20participant"};
    const auto service = CreateService(expectedSimulationId);

    SilKit::Core::EndpointId expectedServiceId = 456;
    auto descriptor = BuildDescriptor(Core::Discovery::controllerTypeRpcServerInternal, expectedServiceId, "my participant");
    descriptor.SetSupplementalDataItem(Core::Discovery::supplKeyRpcServerInternalParentServiceID,
                                       expectedParentServiceId);
    auto dummyRequestDto = ServiceDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateServiceDto(descriptor)).WillOnce(Return(dummyRequestDto));

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualParticipantName;
    oatpp::String actualParentServiceId;
    oatpp::UInt64 actualServiceId;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddRpcServerInternalForParticipantOfSimulation)
        .WillOnce(DoAll(WithArgs<0, 1, 2, 3>([&](auto simulationId, auto participantName, auto parentServiceId, auto serviceId) {
            actualSimulationId = simulationId;
            actualParticipantName = participantName;
            actualParentServiceId = parentServiceId;
            actualServiceId = serviceId;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualParticipantName->c_str(), expectedParticipantName);
    ASSERT_STREQ(actualParentServiceId->c_str(), expectedParentServiceId);
    ASSERT_EQ(actualServiceId, expectedServiceId);
}

TEST_F(TestDashboardSilKitEventHandler, OnServiceDiscoveryEvent_CanLinkCreated_AddCanNetworkToSimulationRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    Core::ServiceDescriptor descriptor;
    descriptor.SetServiceType(Core::ServiceType::Link);
    descriptor.SetNetworkType(Config::NetworkType::CAN);
    descriptor.SetNetworkName("my networkname");

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualNetworkName;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddCanNetworkToSimulation)
        .WillOnce(DoAll(WithArgs<0, 1>([&](auto simulationId, auto networkName) {
            actualSimulationId = simulationId;
            actualNetworkName = networkName;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualNetworkName->c_str(), "my%20networkname");
}

TEST_F(TestDashboardSilKitEventHandler,
       OnServiceDiscoveryEvent_EthernetLinkCreated_AddEthernetNetworkToSimulationRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    Core::ServiceDescriptor descriptor;
    descriptor.SetServiceType(Core::ServiceType::Link);
    descriptor.SetNetworkType(Config::NetworkType::Ethernet);
    descriptor.SetNetworkName("my networkname");

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualNetworkName;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddEthernetNetworkToSimulation)
        .WillOnce(DoAll(WithArgs<0, 1>([&](auto simulationId, auto networkName) {
            actualSimulationId = simulationId;
            actualNetworkName = networkName;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualNetworkName->c_str(), "my%20networkname");
}

TEST_F(TestDashboardSilKitEventHandler,
       OnServiceDiscoveryEvent_FlexrayLinkCreated_AddFlexrayNetworkToSimulationRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    Core::ServiceDescriptor descriptor;
    descriptor.SetServiceType(Core::ServiceType::Link);
    descriptor.SetNetworkType(Config::NetworkType::FlexRay);
    descriptor.SetNetworkName("my networkname");

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualNetworkName;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddFlexrayNetworkToSimulation)
        .WillOnce(DoAll(WithArgs<0, 1>([&](auto simulationId, auto networkName) {
            actualSimulationId = simulationId;
            actualNetworkName = networkName;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualNetworkName->c_str(), "my%20networkname");
}

TEST_F(TestDashboardSilKitEventHandler, OnServiceDiscoveryEvent_LinLinkCreated_AddLinNetworkToSimulationRequestSent)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    Core::ServiceDescriptor descriptor;
    descriptor.SetServiceType(Core::ServiceType::Link);
    descriptor.SetNetworkType(Config::NetworkType::LIN);
    descriptor.SetNetworkName("my networkname");

    oatpp::UInt64 actualSimulationId;
    oatpp::String actualNetworkName;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, AddLinNetworkToSimulation)
        .WillOnce(DoAll(WithArgs<0, 1>([&](auto simulationId, auto networkName) {
            actualSimulationId = simulationId;
            actualNetworkName = networkName;
        })));

    // Act
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated, descriptor);

    // Assert
    ASSERT_EQ(actualSimulationId, expectedSimulationId);
    ASSERT_STREQ(actualNetworkName->c_str(), "my%20networkname");
}

TEST_F(TestDashboardSilKitEventHandler, OnServiceDiscoveryEvent_Invalid_Ignore)
{
    // Arrange & Assert
    const oatpp::UInt64 expectedSimulationId = 123;
    const auto service = CreateService(expectedSimulationId);

    // Act
    Core::ServiceDescriptor descriptor;
    service->OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type::Invalid, descriptor);
}

} // namespace Dashboard
} // namespace SilKit

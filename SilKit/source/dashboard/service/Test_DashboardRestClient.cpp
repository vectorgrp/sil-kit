// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "MockParticipant.hpp"

#include "DashboardRestClient.hpp"

#include "Mocks/MockSilKitToOatppMapper.hpp"
#include "Mocks/MockDashboardSystemServiceClient.hpp"
#include "Mocks/MockDashboardSystemApiClient.hpp"

using namespace testing;
namespace SilKit {
namespace Dashboard {
class Test_DashboardRestClient : public Test
{
public:
    void SetUp() override
    {
        _mockServiceClient = std::make_shared<StrictMock<MockDashboardSystemServiceClient>>();
        _mockSilKitToOatppMapper = std::make_shared<StrictMock<MockSilKitToOatppMapper>>();
        _libraryInit = std::make_shared<LibraryInitializer>();

        EXPECT_CALL(_dummyLogger, GetLogLevel).WillRepeatedly(Return(Services::Logging::Level::Warn));
    }

    std::shared_ptr<DashboardRestClient> CreateService()
    {
        return std::make_shared<DashboardRestClient>(_libraryInit, &_dummyLogger, _mockServiceClient,
                                                     _mockSilKitToOatppMapper);
    }

    Core::Tests::MockLogger _dummyLogger;
    std::shared_ptr<LibraryInitializer> _libraryInit;
    std::shared_ptr<StrictMock<MockDashboardSystemServiceClient>> _mockServiceClient;
    std::shared_ptr<StrictMock<MockSilKitToOatppMapper>> _mockSilKitToOatppMapper;
};

TEST_F(Test_DashboardRestClient, Create)
{
    // Arrange

    // Act
    const auto service = CreateService();

    // Assert
}

TEST_F(Test_DashboardRestClient, OnSimulationStart_CreateSimulationSuccess)
{
    // Arrange
    const uint64_t expectedSimulationId = 123;
    auto request = SimulationCreationRequestDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateSimulationCreationRequestDto).WillOnce(Return(request));
    auto response = SimulationCreationResponseDto::createShared();
    response->id = expectedSimulationId;
    EXPECT_CALL(*_mockServiceClient, CreateSimulation).WillOnce(Return(response));
    const auto service = CreateService();

    // Act
    auto res = service->OnSimulationStart("silkit://localhost:8500", 0);

    // Assert
    ASSERT_EQ(res, expectedSimulationId) << "Wrong simulationId!";
}

TEST_F(Test_DashboardRestClient, OnSimulationStart_CreateSimulationFailure)
{
    // Arrange
    auto request = SimulationCreationRequestDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateSimulationCreationRequestDto).WillOnce(Return(request));
    EXPECT_CALL(*_mockServiceClient, CreateSimulation).WillOnce(Return(nullptr));
    EXPECT_CALL(_dummyLogger, Log(SilKit::Services::Logging::Level::Warn, "Dashboard: creating simulation failed"));
    const auto service = CreateService();

    // Act
    auto res = service->OnSimulationStart("silkit://localhost:8500", 0);

    // Assert
    ASSERT_EQ(res, 0) << "Wrong simulationId!";
}

TEST_F(Test_DashboardRestClient, OnBulkUpdate)
{
    constexpr uint64_t expectedSimulationId{123};
    const auto expectedBulkSimulationDto = SilKit::Dashboard::BulkSimulationDto::createShared();

    // Arrange
    const auto service = CreateService();

    using testing::_;
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateBulkSimulationDto(_)).WillOnce(Return(expectedBulkSimulationDto));

    oatpp::UInt64 simulationId;
    oatpp::Object<BulkSimulationDto> bulkSimulationDto;
    EXPECT_CALL(*_mockServiceClient, UpdateSimulation)
        .WillOnce(WithArgs<0, 1>([&](oatpp::UInt64 simulationId_, oatpp::Object<BulkSimulationDto> bulkSimulation_) {
        simulationId = std::move(simulationId_);
        bulkSimulationDto = std::move(bulkSimulation_);
    }));

    // Act
    service->OnBulkUpdate(expectedSimulationId, SilKit::Dashboard::DashboardBulkUpdate{});

    // Assert
    ASSERT_EQ(simulationId.getValue(0), expectedSimulationId);
    ASSERT_NE(bulkSimulationDto.getPtr(), nullptr);
    ASSERT_EQ(bulkSimulationDto, expectedBulkSimulationDto);
}

} // namespace Dashboard
} // namespace SilKit

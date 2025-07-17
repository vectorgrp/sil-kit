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

class Test_DashboardSilKitEventHandler : public Test
{
public:
    void SetUp() override
    {
        _mockDashboardSystemServiceClient = std::make_shared<StrictMock<MockDashboardSystemServiceClient>>();
        _mockSilKitToOatppMapper = std::make_shared<StrictMock<MockSilKitToOatppMapper>>();

        EXPECT_CALL(_dummyLogger, GetLogLevel).WillRepeatedly(Return(Services::Logging::Level::Warn));
    }

    std::shared_ptr<SilKitEventHandler> CreateService()
    {
        return std::make_shared<SilKitEventHandler>(&_dummyLogger, _mockDashboardSystemServiceClient,
                                                    _mockSilKitToOatppMapper);
    }

    Core::Tests::MockLogger _dummyLogger;
    std::shared_ptr<StrictMock<MockDashboardSystemServiceClient>> _mockDashboardSystemServiceClient;
    std::shared_ptr<StrictMock<MockSilKitToOatppMapper>> _mockSilKitToOatppMapper;
};

TEST_F(Test_DashboardSilKitEventHandler, Create)
{
    // Arrange

    // Act
    const auto service = CreateService();

    // Assert
}

TEST_F(Test_DashboardSilKitEventHandler, OnSimulationStart_CreateSimulationSuccess)
{
    // Arrange
    const uint64_t expectedSimulationId = 123;
    auto request = SimulationCreationRequestDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateSimulationCreationRequestDto).WillOnce(Return(request));
    auto response = SimulationCreationResponseDto::createShared();
    response->id = expectedSimulationId;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, CreateSimulation).WillOnce(Return(response));
    const auto service = CreateService();

    // Act
    auto res = service->OnSimulationStart("silkit://localhost:8500", 0);

    // Assert
    ASSERT_EQ(res, expectedSimulationId) << "Wrong simulationId!";
}

TEST_F(Test_DashboardSilKitEventHandler, OnSimulationStart_CreateSimulationFailure)
{
    // Arrange
    auto request = SimulationCreationRequestDto::createShared();
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateSimulationCreationRequestDto).WillOnce(Return(request));
    EXPECT_CALL(*_mockDashboardSystemServiceClient, CreateSimulation).WillOnce(Return(nullptr));
    EXPECT_CALL(_dummyLogger, Log(SilKit::Services::Logging::Level::Warn, "Dashboard: creating simulation failed"));
    const auto service = CreateService();

    // Act
    auto res = service->OnSimulationStart("silkit://localhost:8500", 0);

    // Assert
    ASSERT_EQ(res, 0) << "Wrong simulationId!";
}

TEST_F(Test_DashboardSilKitEventHandler, OnBulkUpdate)
{
    constexpr uint64_t expectedSimulationId{123};
    const auto expectedBulkSimulationDto = SilKit::Dashboard::BulkSimulationDto::createShared();

    // Arrange
    const auto service = CreateService();

    using testing::_;
    EXPECT_CALL(*_mockSilKitToOatppMapper, CreateBulkSimulationDto(_)).WillOnce(Return(expectedBulkSimulationDto));

    oatpp::UInt64 simulationId;
    oatpp::Object<BulkSimulationDto> bulkSimulationDto;
    EXPECT_CALL(*_mockDashboardSystemServiceClient, UpdateSimulation)
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

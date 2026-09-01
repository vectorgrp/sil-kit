// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "core/mock/participant/MockParticipant.hpp"

#include "dashboard/json/DashboardJson.hpp"
#include "dashboard/service/DashboardRestClient.hpp"

#include "dashboard/client/Mocks/MockDashboardSystemServiceClient.hpp"
#include "dashboard/service/Mocks/MockDashboardDtoMapper.hpp"

using namespace testing;

namespace SilKit {
namespace Dashboard {
namespace {

class Test_DashboardRestClient : public Test
{
public:
    void SetUp() override
    {
        _mockServiceClient = std::make_shared<StrictMock<MockDashboardSystemServiceClient>>();
        _mockDtoMapper = std::make_shared<StrictMock<MockDashboardDtoMapper>>();

        EXPECT_CALL(_dummyLogger, GetLogLevel).WillRepeatedly(Return(Services::Logging::Level::Warn));
    }

    auto CreateService() -> std::shared_ptr<DashboardRestClient>
    {
        return std::make_shared<DashboardRestClient>(&_dummyLogger, _mockServiceClient, _mockDtoMapper);
    }

    Core::Tests::MockLogger _dummyLogger;
    std::shared_ptr<StrictMock<MockDashboardSystemServiceClient>> _mockServiceClient;
    std::shared_ptr<StrictMock<MockDashboardDtoMapper>> _mockDtoMapper;
};

TEST_F(Test_DashboardRestClient, Create)
{
    const auto service = CreateService();
}

TEST_F(Test_DashboardRestClient, OnSimulationStart_CreateSimulationSuccess)
{
    constexpr uint64_t expectedSimulationId = 123;
    EXPECT_CALL(*_mockDtoMapper, CreateSimulationCreationRequestDto)
        .WillOnce(Return(SimulationCreationRequestDto{}));
    EXPECT_CALL(*_mockServiceClient, CreateSimulation).WillOnce(Return(expectedSimulationId));

    const auto service = CreateService();
    const auto simulationId = service->OnSimulationStart("silkit://localhost:8500", 0);

    ASSERT_EQ(simulationId, expectedSimulationId) << "Wrong simulationId!";
}

TEST_F(Test_DashboardRestClient, OnSimulationStart_CreateSimulationFailure)
{
    EXPECT_CALL(*_mockDtoMapper, CreateSimulationCreationRequestDto)
        .WillOnce(Return(SimulationCreationRequestDto{}));
    EXPECT_CALL(*_mockServiceClient, CreateSimulation).WillOnce(Return(std::nullopt));
    EXPECT_CALL(_dummyLogger, ProcessLoggerMessage(Services::Logging::ALoggerMessageWith(
                                  Services::Logging::Level::Warn, "Dashboard: creating simulation failed")));

    const auto service = CreateService();
    const auto simulationId = service->OnSimulationStart("silkit://localhost:8500", 0);

    ASSERT_EQ(simulationId, 0u) << "Wrong simulationId!";
}

TEST_F(Test_DashboardRestClient, OnBulkUpdate_ForwardsTheMappedDtoWithTheSimulationId)
{
    constexpr uint64_t expectedSimulationId{123};

    // The DTOs are plain aggregates without operator==, so compare their serialized form; that also
    // covers the writer for this payload.
    BulkSimulationDto expectedDto{};
    expectedDto.stopped = 42;

    EXPECT_CALL(*_mockDtoMapper, CreateBulkSimulationDto(_)).WillOnce(Return(expectedDto));

    uint64_t actualSimulationId{0};
    std::string actualDtoJson;
    EXPECT_CALL(*_mockServiceClient, UpdateSimulation)
        .WillOnce(WithArgs<0, 1>([&](uint64_t simulationId, const BulkSimulationDto& bulkSimulation) {
        actualSimulationId = simulationId;
        actualDtoJson = ToJson(bulkSimulation);
    }));

    const auto service = CreateService();
    service->OnBulkUpdate(expectedSimulationId, DashboardBulkUpdate{});

    EXPECT_EQ(actualSimulationId, expectedSimulationId);
    EXPECT_EQ(actualDtoJson, ToJson(expectedDto));
}

TEST_F(Test_DashboardRestClient, OnMetricsUpdate_ForwardsTheMappedDtoWithTheSimulationId)
{
    constexpr uint64_t expectedSimulationId{7};

    MetricsUpdateDto expectedDto{};
    CounterDataDto counter{};
    counter.ts = 1;
    counter.pn = "P1";
    counter.mn = {"c"};
    counter.mv = 5;
    expectedDto.counters.push_back(counter);

    EXPECT_CALL(*_mockDtoMapper, CreateMetricsUpdateDto("P1", _)).WillOnce(Return(expectedDto));

    uint64_t actualSimulationId{0};
    std::string actualDtoJson;
    EXPECT_CALL(*_mockServiceClient, UpdateSimulationMetrics)
        .WillOnce(WithArgs<0, 1>([&](uint64_t simulationId, const MetricsUpdateDto& metrics) {
        actualSimulationId = simulationId;
        actualDtoJson = ToJson(metrics);
    }));

    const auto service = CreateService();
    service->OnMetricsUpdate(expectedSimulationId, "P1", VSilKit::MetricsUpdate{});

    EXPECT_EQ(actualSimulationId, expectedSimulationId);
    EXPECT_EQ(actualDtoJson, ToJson(expectedDto));
}

TEST_F(Test_DashboardRestClient, IsBulkUpdateSupported_DelegatesToTheServiceClient)
{
    EXPECT_CALL(*_mockServiceClient, CheckBulkUpdateSupported()).WillOnce(Return(true));

    const auto service = CreateService();

    EXPECT_TRUE(service->IsBulkUpdateSupported());
}

// The test constructor has no transport, so Abort() must still be safe to call.
TEST_F(Test_DashboardRestClient, Abort_WithoutATransport_IsANoOp)
{
    const auto service = CreateService();

    service->Abort();
    service->Abort();
}

} // namespace
} // namespace Dashboard
} // namespace SilKit

// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "core/mock/participant/MockParticipant.hpp"

#include "dashboard/client/DashboardPaths.hpp"
#include "dashboard/client/DashboardSystemServiceClient.hpp"
#include "dashboard/http/Mocks/MockHttpClient.hpp"
#include "dashboard/json/DashboardJson.hpp"

using namespace testing;
using VSilKit::HttpResult;
using VSilKit::MockHttpClient;

namespace SilKit {
namespace Dashboard {
namespace {

auto Responded(int statusCode, std::string body = {}) -> HttpResult
{
    return HttpResult{false, statusCode, std::move(body)};
}

auto Unavailable() -> HttpResult
{
    return HttpResult{};
}

class Test_DashboardSystemServiceClient : public Test
{
public:
    void SetUp() override
    {
        _mockHttpClient = std::make_shared<StrictMock<MockHttpClient>>();
        EXPECT_CALL(_dummyLogger, GetLogLevel).WillRepeatedly(Return(Services::Logging::Level::Debug));
    }

    auto CreateService() -> std::shared_ptr<IDashboardSystemServiceClient>
    {
        return std::make_shared<DashboardSystemServiceClient>(&_dummyLogger, _mockHttpClient);
    }

    void ExpectLog(Services::Logging::Level level, const std::string& message)
    {
        EXPECT_CALL(_dummyLogger, ProcessLoggerMessage(Services::Logging::ALoggerMessageWith(level, message)));
    }

    Core::Tests::MockLogger _dummyLogger;
    std::shared_ptr<StrictMock<MockHttpClient>> _mockHttpClient;
};

// --- CreateSimulation -------------------------------------------------------------------------

TEST_F(Test_DashboardSystemServiceClient, CreateSimulation_Success)
{
    EXPECT_CALL(*_mockHttpClient, Post("system-service/v1.0/simulations", _))
        .WillOnce(Return(Responded(201, R"({"id":123})")));
    ExpectLog(Services::Logging::Level::Debug, "Dashboard: creating simulation returned 201");

    const auto service = CreateService();
    const auto simulationId = service->CreateSimulation(SimulationCreationRequestDto{});

    ASSERT_TRUE(simulationId.has_value());
    EXPECT_EQ(*simulationId, 123u);
}

TEST_F(Test_DashboardSystemServiceClient, CreateSimulation_ServerError)
{
    EXPECT_CALL(*_mockHttpClient, Post("system-service/v1.0/simulations", _)).WillOnce(Return(Responded(500)));
    ExpectLog(Services::Logging::Level::Error, "Dashboard: creating simulation returned 500");

    const auto service = CreateService();

    EXPECT_FALSE(service->CreateSimulation(SimulationCreationRequestDto{}).has_value());
}

TEST_F(Test_DashboardSystemServiceClient, CreateSimulation_TransportFailure_LogsServerUnavailable)
{
    EXPECT_CALL(*_mockHttpClient, Post(_, _)).WillOnce(Return(Unavailable()));
    ExpectLog(Services::Logging::Level::Error, "Dashboard: creating simulation server unavailable");

    const auto service = CreateService();

    EXPECT_FALSE(service->CreateSimulation(SimulationCreationRequestDto{}).has_value());
}

// A 201 whose body we cannot read is treated as "no id", not as a hard failure.
TEST_F(Test_DashboardSystemServiceClient, CreateSimulation_UnparsableBody)
{
    EXPECT_CALL(*_mockHttpClient, Post(_, _)).WillOnce(Return(Responded(201, "not json")));
    ExpectLog(Services::Logging::Level::Debug, "Dashboard: creating simulation returned 201");

    const auto service = CreateService();

    EXPECT_FALSE(service->CreateSimulation(SimulationCreationRequestDto{}).has_value());
}

TEST_F(Test_DashboardSystemServiceClient, CreateSimulation_SendsTheSerializedRequest)
{
    SimulationCreationRequestDto request{};
    request.started = 17;
    request.configuration.connectUri = "silkit://localhost:8500";

    std::string actualBody;
    EXPECT_CALL(*_mockHttpClient, Post(Paths::CreateSimulation(), _))
        .WillOnce(DoAll(SaveArg<1>(&actualBody), Return(Responded(201, R"({"id":1})"))));
    ExpectLog(Services::Logging::Level::Debug, "Dashboard: creating simulation returned 201");

    const auto service = CreateService();
    service->CreateSimulation(request);

    EXPECT_EQ(actualBody, ToJson(request));
    EXPECT_EQ(actualBody,
              "{\"started\": 17,\"configuration\": {\"connectUri\": \"silkit://localhost:8500\"}}");
}

// --- UpdateSimulation -------------------------------------------------------------------------

TEST_F(Test_DashboardSystemServiceClient, UpdateSimulation_UsesTheSimulationIdInThePath)
{
    std::string actualBody;
    EXPECT_CALL(*_mockHttpClient, Post("system-service/v1.1/simulations/456", _))
        .WillOnce(DoAll(SaveArg<1>(&actualBody), Return(Responded(200))));
    ExpectLog(Services::Logging::Level::Debug, "Dashboard: updating simulation returned 200");

    const auto service = CreateService();
    service->UpdateSimulation(456, BulkSimulationDto{});

    EXPECT_EQ(actualBody, ToJson(BulkSimulationDto{}));
}

TEST_F(Test_DashboardSystemServiceClient, UpdateSimulation_TransportFailure)
{
    EXPECT_CALL(*_mockHttpClient, Post(_, _)).WillOnce(Return(Unavailable()));
    ExpectLog(Services::Logging::Level::Error, "Dashboard: updating simulation server unavailable");

    const auto service = CreateService();
    service->UpdateSimulation(1, BulkSimulationDto{});
}

// --- UpdateSimulationMetrics ------------------------------------------------------------------

TEST_F(Test_DashboardSystemServiceClient, UpdateSimulationMetrics_UsesTheMetricsPath)
{
    std::string actualBody;
    EXPECT_CALL(*_mockHttpClient, Post("system-service/v1.1/simulations/789/metrics", _))
        .WillOnce(DoAll(SaveArg<1>(&actualBody), Return(Responded(200))));
    ExpectLog(Services::Logging::Level::Debug, "Dashboard: updating simulation metrics returned 200");

    const auto service = CreateService();
    service->UpdateSimulationMetrics(789, MetricsUpdateDto{});

    EXPECT_EQ(actualBody, ToJson(MetricsUpdateDto{}));
}

// --- CheckBulkUpdateSupported -----------------------------------------------------------------

/*! Probes the bulk endpoint with simulation id 0 and an empty payload, and deliberately does not
 *  log: the previous implementation issued this request through the raw api client, bypassing the
 *  logging wrapper, and the caller reports the outcome instead.
 */
TEST_F(Test_DashboardSystemServiceClient, CheckBulkUpdateSupported_SuccessStatusMeansSupported)
{
    std::string actualBody;
    EXPECT_CALL(*_mockHttpClient, Post("system-service/v1.1/simulations/0", _))
        .WillOnce(DoAll(SaveArg<1>(&actualBody), Return(Responded(200))));

    const auto service = CreateService();

    EXPECT_TRUE(service->CheckBulkUpdateSupported());
    EXPECT_EQ(actualBody, "{\"stopped\": null,\"system\": {\"statuses\": []},\"participants\": []}");
}

TEST_F(Test_DashboardSystemServiceClient, CheckBulkUpdateSupported_NonSuccessStatusMeansUnsupported)
{
    EXPECT_CALL(*_mockHttpClient, Post(_, _)).WillOnce(Return(Responded(404)));

    const auto service = CreateService();

    EXPECT_FALSE(service->CheckBulkUpdateSupported());
}

TEST_F(Test_DashboardSystemServiceClient, CheckBulkUpdateSupported_TransportFailureMeansUnsupported)
{
    EXPECT_CALL(*_mockHttpClient, Post(_, _)).WillOnce(Return(Unavailable()));

    const auto service = CreateService();

    EXPECT_FALSE(service->CheckBulkUpdateSupported());
}

} // namespace
} // namespace Dashboard
} // namespace SilKit

// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "gmock/gmock.h"

#include "dashboard/client/IDashboardSystemServiceClient.hpp"

namespace SilKit {
namespace Dashboard {

class MockDashboardSystemServiceClient : public IDashboardSystemServiceClient
{
public:
    MOCK_METHOD(std::optional<uint64_t>, CreateSimulation, (const SimulationCreationRequestDto& simulation),
                (override));
    MOCK_METHOD(void, UpdateSimulation, (uint64_t simulationId, const BulkSimulationDto& bulkSimulation), (override));
    MOCK_METHOD(void, UpdateSimulationMetrics, (uint64_t simulationId, const MetricsUpdateDto& metrics), (override));
    MOCK_METHOD(bool, CheckBulkUpdateSupported, (), (override));
};

} // namespace Dashboard
} // namespace SilKit

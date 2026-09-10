// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "gmock/gmock.h"

#include "dashboard/DashboardBulkUpdate.hpp"
#include "dashboard/IRestClient.hpp"
#include "services/metrics/MetricsDatatypes.hpp"

namespace VSilKit {

class MockRestClient : public IRestClient
{
public:
    MOCK_METHOD(uint64_t, OnSimulationStart, (const std::string& connectUri, uint64_t time), (override));
    MOCK_METHOD(void, OnBulkUpdate, (uint64_t simulationId, const SilKit::Dashboard::DashboardBulkUpdate& bulkUpdate),
                (override));
    MOCK_METHOD(void, OnMetricsUpdate,
                (uint64_t simulationId, const std::string& origin, const VSilKit::MetricsUpdate& metricsUpdate),
                (override));
    MOCK_METHOD(void, Abort, (), (override));
};

} // namespace VSilKit

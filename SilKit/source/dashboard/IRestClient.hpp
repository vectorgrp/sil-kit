// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <cstdint>

namespace VSilKit {

class DashboardBulkUpdate;
class MetricsUpdate;

class IRestClient
{
public:
    virtual ~IRestClient() = default;

    virtual uint64_t OnSimulationStart(const std::string& connectUri, uint64_t time) = 0;
    virtual void OnBulkUpdate(uint64_t simulationId, const DashboardBulkUpdate& bulkUpdate) = 0;
    virtual void OnMetricsUpdate(uint64_t simulationId, const std::string& origin, const VSilKit::MetricsUpdate& metricsUpdate) = 0;
    virtual bool IsBulkUpdateSupported() const = 0;
};

} // namespace VSilKit

// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "gmock/gmock.h"

#include "dashboard/service/IDashboardDtoMapper.hpp"

namespace SilKit {
namespace Dashboard {

class MockDashboardDtoMapper : public IDashboardDtoMapper
{
public:
    MOCK_METHOD(SimulationCreationRequestDto, CreateSimulationCreationRequestDto,
                (const std::string& connectUri, uint64_t start), (override));
    MOCK_METHOD(BulkSimulationDto, CreateBulkSimulationDto, (const DashboardBulkUpdate& bulkUpdate), (override));
    MOCK_METHOD(MetricsUpdateDto, CreateMetricsUpdateDto,
                (const std::string& participantName, const VSilKit::MetricsUpdate& metricsUpdate), (override));
};

} // namespace Dashboard
} // namespace SilKit

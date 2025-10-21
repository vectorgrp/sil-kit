// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "gmock/gmock-function-mocker.h"

#include "IDashboardSystemServiceClient.hpp"

namespace SilKit {
namespace Dashboard {
class MockDashboardSystemServiceClient : public IDashboardSystemServiceClient
{
public:
    MOCK_METHOD(oatpp::Object<SimulationCreationResponseDto>, CreateSimulation,
                (oatpp::Object<SimulationCreationRequestDto>), (override));

    MOCK_METHOD(void, UpdateSimulation, (oatpp::UInt64, oatpp::Object<BulkSimulationDto>), (override));

    MOCK_METHOD(void, UpdateSimulationMetrics, (oatpp::UInt64, oatpp::Object<MetricsUpdateDto>), (override));
};
} // namespace Dashboard
} // namespace SilKit

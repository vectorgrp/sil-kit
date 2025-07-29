// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "gmock/gmock-function-mocker.h"
#include "ISilKitToOatppMapper.hpp"

namespace SilKit {
namespace Dashboard {
class MockSilKitToOatppMapper : public ISilKitToOatppMapper
{
    using ServiceDescriptor = SilKit::Core::ServiceDescriptor;

    template <typename T>
    using Object = oatpp::Object<T>;

public:
    MOCK_METHOD(oatpp::Object<SilKit::Dashboard::SimulationCreationRequestDto>, CreateSimulationCreationRequestDto,
                (const std::string&, uint64_t), (override));
    MOCK_METHOD(oatpp::Object<SilKit::Dashboard::SystemStatusDto>, CreateSystemStatusDto,
                (SilKit::Services::Orchestration::SystemState), (override));
    MOCK_METHOD(oatpp::Object<SilKit::Dashboard::ParticipantStatusDto>, CreateParticipantStatusDto,
                (const SilKit::Services::Orchestration::ParticipantStatus&), (override));
    MOCK_METHOD(oatpp::Object<SilKit::Dashboard::ServiceDto>, CreateServiceDto,
                (const SilKit::Core::ServiceDescriptor&), (override));

    MOCK_METHOD(Object<BulkControllerDto>, CreateBulkControllerDto, (const ServiceDescriptor&), (override));
    MOCK_METHOD(Object<BulkDataServiceDto>, CreateBulkDataServiceDto, (const ServiceDescriptor&), (override));
    MOCK_METHOD(Object<BulkRpcServiceDto>, CreateBulkRpcServiceDto, (const ServiceDescriptor&), (override));
    MOCK_METHOD(Object<BulkServiceInternalDto>, CreateBulkServiceInternalDto, (const ServiceDescriptor&), (override));
    MOCK_METHOD(Object<BulkSimulationDto>, CreateBulkSimulationDto, (const DashboardBulkUpdate&), (override));
    MOCK_METHOD(Object<MetricsUpdateDto>, CreateMetricsUpdateDto, (const std::string&, const VSilKit::MetricsUpdate&),
                (override));
};
} // namespace Dashboard
} // namespace SilKit
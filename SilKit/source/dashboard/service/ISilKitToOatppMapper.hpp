// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

#include "OatppHeaders.hpp"

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "ParticipantStatusDto.hpp"
#include "ServiceDescriptor.hpp"
#include "ServiceDto.hpp"
#include "SimulationCreationRequestDto.hpp"
#include "SystemStatusDto.hpp"
#include "BulkUpdateDto.hpp"
#include "MetricsDto.hpp"

#include "DashboardBulkUpdate.hpp"
#include "MetricsDatatypes.hpp"


namespace SilKit {
namespace Dashboard {
class ISilKitToOatppMapper
{
    using ServiceDescriptor = SilKit::Core::ServiceDescriptor;

    template <typename T>
    using Object = oatpp::Object<T>;

public:
    virtual ~ISilKitToOatppMapper() = default;
    virtual oatpp::Object<SimulationCreationRequestDto> CreateSimulationCreationRequestDto(
        const std::string& connectUri, uint64_t start) = 0;
    virtual oatpp::Object<SystemStatusDto> CreateSystemStatusDto(Services::Orchestration::SystemState systemState) = 0;
    virtual oatpp::Object<ParticipantStatusDto> CreateParticipantStatusDto(
        const Services::Orchestration::ParticipantStatus& participantStatus) = 0;
    virtual oatpp::Object<ServiceDto> CreateServiceDto(const Core::ServiceDescriptor& serviceDescriptor) = 0;

    virtual auto CreateBulkControllerDto(const ServiceDescriptor& serviceDescriptor) -> Object<BulkControllerDto> = 0;
    virtual auto CreateBulkDataServiceDto(const ServiceDescriptor& serviceDescriptor) -> Object<BulkDataServiceDto> = 0;
    virtual auto CreateBulkRpcServiceDto(const ServiceDescriptor& serviceDescriptor) -> Object<BulkRpcServiceDto> = 0;
    virtual auto CreateBulkServiceInternalDto(const ServiceDescriptor& serviceDescriptor)
        -> Object<BulkServiceInternalDto> = 0;
    virtual auto CreateBulkSimulationDto(const DashboardBulkUpdate& bulkUpdate) -> Object<BulkSimulationDto> = 0;
    virtual auto CreateMetricsUpdateDto(const std::string& origin,
                                        const VSilKit::MetricsUpdate& metricsUpdate) -> Object<MetricsUpdateDto> = 0;
};
} // namespace Dashboard
} // namespace SilKit

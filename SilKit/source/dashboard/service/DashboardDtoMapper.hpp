// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/internal/ServiceDescriptor.hpp"

#include "dashboard/service/IDashboardDtoMapper.hpp"

namespace SilKit {
namespace Dashboard {

class DashboardDtoMapper : public IDashboardDtoMapper
{
    using ServiceDescriptor = SilKit::Core::ServiceDescriptor;

public:
    auto CreateSimulationCreationRequestDto(const std::string& connectUri,
                                            uint64_t start) -> SimulationCreationRequestDto override;
    auto CreateBulkSimulationDto(const DashboardBulkUpdate& bulkUpdate) -> BulkSimulationDto override;
    auto CreateMetricsUpdateDto(const std::string& participantName,
                                const VSilKit::MetricsUpdate& metricsUpdate) -> MetricsUpdateDto override;

public: // exercised directly by the tests
    auto CreateSystemStatusDto(Services::Orchestration::SystemState systemState) -> SystemStatusDto;
    auto CreateParticipantStatusDto(const Services::Orchestration::ParticipantStatus& participantStatus)
        -> ParticipantStatusDto;
    auto CreateBulkControllerDto(const ServiceDescriptor& serviceDescriptor) -> BulkControllerDto;
    auto CreateBulkDataServiceDto(const ServiceDescriptor& serviceDescriptor) -> BulkDataServiceDto;
    auto CreateBulkRpcServiceDto(const ServiceDescriptor& serviceDescriptor) -> BulkRpcServiceDto;
    auto CreateBulkServiceInternalDto(const ServiceDescriptor& serviceDescriptor) -> BulkServiceInternalDto;

private:
    void ProcessServiceDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor);
    void ProcessControllerDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor);
    void ProcessLinkDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor);
};

} // namespace Dashboard
} // namespace SilKit

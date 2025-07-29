// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "ISilKitToOatppMapper.hpp"

namespace SilKit {
namespace Dashboard {

class SilKitToOatppMapper : public ISilKitToOatppMapper
{
    using ServiceDescriptor = SilKit::Core::ServiceDescriptor;

    template <typename T>
    using Object = oatpp::Object<T>;

public:
    oatpp::Object<SimulationCreationRequestDto> CreateSimulationCreationRequestDto(const std::string& connectUri,
                                                                                   uint64_t start) override;
    oatpp::Object<SystemStatusDto> CreateSystemStatusDto(Services::Orchestration::SystemState systemState) override;
    oatpp::Object<ParticipantStatusDto> CreateParticipantStatusDto(
        const Services::Orchestration::ParticipantStatus& participantStatus) override;
    oatpp::Object<ServiceDto> CreateServiceDto(const Core::ServiceDescriptor& serviceDescriptor) override;

    auto CreateBulkControllerDto(const ServiceDescriptor& serviceDescriptor) -> Object<BulkControllerDto> override;
    auto CreateBulkDataServiceDto(const ServiceDescriptor& serviceDescriptor) -> Object<BulkDataServiceDto> override;
    auto CreateBulkRpcServiceDto(const ServiceDescriptor& serviceDescriptor) -> Object<BulkRpcServiceDto> override;
    auto CreateBulkServiceInternalDto(const ServiceDescriptor& serviceDescriptor)
        -> Object<BulkServiceInternalDto> override;
    auto CreateBulkSimulationDto(const DashboardBulkUpdate& bulkUpdate) -> Object<BulkSimulationDto> override;
    auto CreateMetricsUpdateDto(const std::string& origin,
                                const VSilKit::MetricsUpdate& metricsUpdate) -> Object<MetricsUpdateDto> override;

private:
    void ProcessServiceDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor);
    void ProcessControllerDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor);
    void ProcessLinkDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor);
};

} // namespace Dashboard
} // namespace SilKit

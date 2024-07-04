/* Copyright (c) 2022 Vector Informatik GmbH
 
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
    oatpp::Object<DataPublisherDto> CreateDataPublisherDto(const Core::ServiceDescriptor& serviceDescriptor) override;
    oatpp::Object<DataSubscriberDto> CreateDataSubscriberDto(const Core::ServiceDescriptor& serviceDescriptor) override;
    oatpp::Object<RpcClientDto> CreateRpcClientDto(const Core::ServiceDescriptor& serviceDescriptor) override;
    oatpp::Object<RpcServerDto> CreateRpcServerDto(const Core::ServiceDescriptor& serviceDescriptor) override;
    oatpp::Object<SimulationEndDto> CreateSimulationEndDto(uint64_t stop) override;

    auto CreateBulkControllerDto(const ServiceDescriptor& serviceDescriptor) -> Object<BulkControllerDto> override;
    auto CreateBulkDataServiceDto(const ServiceDescriptor& serviceDescriptor) -> Object<BulkDataServiceDto> override;
    auto CreateBulkRpcServiceDto(const ServiceDescriptor& serviceDescriptor) -> Object<BulkRpcServiceDto> override;
    auto CreateBulkServiceInternalDto(const ServiceDescriptor& serviceDescriptor)
        -> Object<BulkServiceInternalDto> override;
    auto CreateBulkSimulationDto(const DashboardBulkUpdate& bulkUpdate) -> Object<BulkSimulationDto> override;

private:
    void ProcessServiceDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor);
    void ProcessControllerDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor);
    void ProcessLinkDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor);
};

} // namespace Dashboard
} // namespace SilKit

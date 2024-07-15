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

#include <cstdint>

#include "OatppHeaders.hpp"

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "DataPublisherDto.hpp"
#include "DataSubscriberDto.hpp"
#include "ParticipantStatusDto.hpp"
#include "RpcClientDto.hpp"
#include "RpcServerDto.hpp"
#include "ServiceDescriptor.hpp"
#include "ServiceDto.hpp"
#include "SimulationCreationRequestDto.hpp"
#include "SimulationEndDto.hpp"
#include "SystemStatusDto.hpp"
#include "BulkUpdateDto.hpp"

#include "DashboardBulkUpdate.hpp"


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
    virtual oatpp::Object<DataPublisherDto> CreateDataPublisherDto(
        const Core::ServiceDescriptor& serviceDescriptor) = 0;
    virtual oatpp::Object<DataSubscriberDto> CreateDataSubscriberDto(
        const Core::ServiceDescriptor& serviceDescriptor) = 0;
    virtual oatpp::Object<RpcClientDto> CreateRpcClientDto(const Core::ServiceDescriptor& serviceDescriptor) = 0;
    virtual oatpp::Object<RpcServerDto> CreateRpcServerDto(const Core::ServiceDescriptor& serviceDescriptor) = 0;
    virtual oatpp::Object<SimulationEndDto> CreateSimulationEndDto(uint64_t stop) = 0;

    virtual auto CreateBulkControllerDto(const ServiceDescriptor& serviceDescriptor) -> Object<BulkControllerDto> = 0;
    virtual auto CreateBulkDataServiceDto(const ServiceDescriptor& serviceDescriptor) -> Object<BulkDataServiceDto> = 0;
    virtual auto CreateBulkRpcServiceDto(const ServiceDescriptor& serviceDescriptor) -> Object<BulkRpcServiceDto> = 0;
    virtual auto CreateBulkServiceInternalDto(const ServiceDescriptor& serviceDescriptor)
        -> Object<BulkServiceInternalDto> = 0;
    virtual auto CreateBulkSimulationDto(const DashboardBulkUpdate& bulkUpdate) -> Object<BulkSimulationDto> = 0;
};
} // namespace Dashboard
} // namespace SilKit

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
    MOCK_METHOD(oatpp::Object<SilKit::Dashboard::DataPublisherDto>, CreateDataPublisherDto,
                (const SilKit::Core::ServiceDescriptor&), (override));
    MOCK_METHOD(oatpp::Object<SilKit::Dashboard::DataSubscriberDto>, CreateDataSubscriberDto,
                (const SilKit::Core::ServiceDescriptor&), (override));
    MOCK_METHOD(oatpp::Object<SilKit::Dashboard::RpcClientDto>, CreateRpcClientDto,
                (const SilKit::Core::ServiceDescriptor&), (override));
    MOCK_METHOD(oatpp::Object<SilKit::Dashboard::RpcServerDto>, CreateRpcServerDto,
                (const SilKit::Core::ServiceDescriptor&), (override));
    MOCK_METHOD(oatpp::Object<SimulationEndDto>, CreateSimulationEndDto, (uint64_t), (override));

    MOCK_METHOD(Object<BulkControllerDto>, CreateBulkControllerDto, (const ServiceDescriptor&), (override));
    MOCK_METHOD(Object<BulkDataServiceDto>, CreateBulkDataServiceDto, (const ServiceDescriptor&), (override));
    MOCK_METHOD(Object<BulkRpcServiceDto>, CreateBulkRpcServiceDto, (const ServiceDescriptor&), (override));
    MOCK_METHOD(Object<BulkServiceInternalDto>, CreateBulkServiceInternalDto, (const ServiceDescriptor&), (override));
    MOCK_METHOD(Object<BulkSimulationDto>, CreateBulkSimulationDto, (const DashboardBulkUpdate&), (override));
};
} // namespace Dashboard
} // namespace SilKit
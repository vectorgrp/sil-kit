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

#include "IDashboardSystemServiceClient.hpp"

namespace SilKit {
namespace Dashboard {
class MockDashboardSystemServiceClient : public IDashboardSystemServiceClient
{
public:
    MockDashboardSystemServiceClient() {}

    MOCK_METHOD(oatpp::Object<SimulationCreationResponseDto>, CreateSimulation,
                (oatpp::Object<SimulationCreationRequestDto>), (override));

    MOCK_METHOD(void, UpdateSimulation, (oatpp::UInt64, oatpp::Object<BulkSimulationDto>), (override));

    MOCK_METHOD(void, AddParticipantToSimulation, (oatpp::UInt64, oatpp::String), (override));

    MOCK_METHOD(void, AddParticipantStatusForSimulation,
                (oatpp::UInt64, oatpp::String, oatpp::Object<ParticipantStatusDto>), (override));

    MOCK_METHOD(void, AddCanControllerForParticipantOfSimulation,
                (oatpp::UInt64, oatpp::String, oatpp::UInt64, oatpp::Object<ServiceDto>), (override));

    MOCK_METHOD(void, AddEthernetControllerForParticipantOfSimulation,
                (oatpp::UInt64, oatpp::String, oatpp::UInt64, oatpp::Object<ServiceDto>), (override));

    MOCK_METHOD(void, AddFlexrayControllerForParticipantOfSimulation,
                (oatpp::UInt64, oatpp::String, oatpp::UInt64, oatpp::Object<ServiceDto>), (override));

    MOCK_METHOD(void, AddLinControllerForParticipantOfSimulation,
                (oatpp::UInt64, oatpp::String, oatpp::UInt64, oatpp::Object<ServiceDto>), (override));

    MOCK_METHOD(void, AddDataPublisherForParticipantOfSimulation,
                (oatpp::UInt64, oatpp::String, oatpp::UInt64, oatpp::Object<DataPublisherDto>), (override));

    MOCK_METHOD(void, AddDataSubscriberForParticipantOfSimulation,
                (oatpp::UInt64, oatpp::String, oatpp::UInt64, oatpp::Object<DataSubscriberDto>), (override));

    MOCK_METHOD(void, AddDataSubscriberInternalForParticipantOfSimulation,
                (oatpp::UInt64, oatpp::String, oatpp::String, oatpp::UInt64, oatpp::Object<ServiceDto>), (override));

    MOCK_METHOD(void, AddRpcClientForParticipantOfSimulation,
                (oatpp::UInt64, oatpp::String, oatpp::UInt64, oatpp::Object<RpcClientDto>), (override));

    MOCK_METHOD(void, AddRpcServerForParticipantOfSimulation,
                (oatpp::UInt64, oatpp::String, oatpp::UInt64, oatpp::Object<RpcServerDto>), (override));

    MOCK_METHOD(void, AddRpcServerInternalForParticipantOfSimulation,
                (oatpp::UInt64, oatpp::String, oatpp::String, oatpp::UInt64, oatpp::Object<ServiceDto>), (override));

    MOCK_METHOD(void, AddCanNetworkToSimulation, (oatpp::UInt64, oatpp::String, oatpp::String), (override));

    MOCK_METHOD(void, AddEthernetNetworkToSimulation, (oatpp::UInt64, oatpp::String, oatpp::String), (override));

    MOCK_METHOD(void, AddFlexrayNetworkToSimulation, (oatpp::UInt64, oatpp::String, oatpp::String), (override));

    MOCK_METHOD(void, AddLinNetworkToSimulation, (oatpp::UInt64, oatpp::String, oatpp::String), (override));

    MOCK_METHOD(void, UpdateSystemStatusForSimulation, (oatpp::UInt64, oatpp::Object<SystemStatusDto>), (override));

    MOCK_METHOD(void, SetSimulationEnd, (oatpp::UInt64, oatpp::Object<SimulationEndDto>), (override));
};
} // namespace Dashboard
} // namespace SilKit

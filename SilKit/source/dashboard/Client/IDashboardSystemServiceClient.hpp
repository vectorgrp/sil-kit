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

#include "SimulationCreationRequestDto.hpp"
#include "SimulationCreationResponseDto.hpp"
#include "SystemStatusDto.hpp"
#include "ParticipantStatusDto.hpp"
#include "ServiceDto.hpp"
#include "DataPublisherDto.hpp"
#include "DataSubscriberDto.hpp"
#include "RpcClientDto.hpp"
#include "RpcServerDto.hpp"
#include "SimulationEndDto.hpp"
#include "BulkUpdateDto.hpp"

namespace SilKit {
namespace Dashboard {

class IDashboardSystemServiceClient
{
public:
    virtual ~IDashboardSystemServiceClient() = default;

    virtual oatpp::Object<SimulationCreationResponseDto> CreateSimulation(
        oatpp::Object<SimulationCreationRequestDto> simulation) = 0;

    virtual void UpdateSimulation(oatpp::UInt64 simulationId, oatpp::Object<BulkSimulationDto> bulkSimulation) = 0;

    virtual void AddParticipantToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName) = 0;

    virtual void AddParticipantStatusForSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                   oatpp::Object<ParticipantStatusDto> participantStatus) = 0;

    virtual void AddCanControllerForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                            oatpp::UInt64 serviceId,
                                                            oatpp::Object<ServiceDto> canController) = 0;

    virtual void AddEthernetControllerForParticipantOfSimulation(oatpp::UInt64 simulationId,
                                                                 oatpp::String participantName, oatpp::UInt64 serviceId,
                                                                 oatpp::Object<ServiceDto> ethernetController) = 0;

    virtual void AddFlexrayControllerForParticipantOfSimulation(oatpp::UInt64 simulationId,
                                                                oatpp::String participantName, oatpp::UInt64 serviceId,
                                                                oatpp::Object<ServiceDto> flexrayController) = 0;

    virtual void AddLinControllerForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                            oatpp::UInt64 serviceId,
                                                            oatpp::Object<ServiceDto> linController) = 0;

    virtual void AddDataPublisherForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                            oatpp::UInt64 serviceId,
                                                            oatpp::Object<DataPublisherDto> dataPublisher) = 0;

    virtual void AddDataSubscriberForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                             oatpp::UInt64 serviceId,
                                                             oatpp::Object<DataSubscriberDto> dataSubscriber) = 0;

    virtual void AddDataSubscriberInternalForParticipantOfSimulation(
        oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::String parentServiceId,
        oatpp::UInt64 serviceId, oatpp::Object<ServiceDto> dataSubscriberInternal) = 0;

    virtual void AddRpcClientForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                        oatpp::UInt64 serviceId,
                                                        oatpp::Object<RpcClientDto> rpcClient) = 0;

    virtual void AddRpcServerForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                        oatpp::UInt64 serviceId,
                                                        oatpp::Object<RpcServerDto> rpcServer) = 0;

    virtual void AddRpcServerInternalForParticipantOfSimulation(oatpp::UInt64 simulationId,
                                                                oatpp::String participantName,
                                                                oatpp::String parentServiceId, oatpp::UInt64 serviceId,
                                                                oatpp::Object<ServiceDto> rpcServerInternal) = 0;

    virtual void AddCanNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                           oatpp::String networkName) = 0;

    virtual void AddEthernetNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                oatpp::String networkName) = 0;

    virtual void AddFlexrayNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                               oatpp::String networkName) = 0;

    virtual void AddLinNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                           oatpp::String networkName) = 0;

    virtual void UpdateSystemStatusForSimulation(oatpp::UInt64 simulationId,
                                                 oatpp::Object<SystemStatusDto> systemStatus) = 0;

    virtual void SetSimulationEnd(oatpp::UInt64 simulationId, oatpp::Object<SimulationEndDto> simulation) = 0;
};

} // namespace Dashboard
} // namespace SilKit

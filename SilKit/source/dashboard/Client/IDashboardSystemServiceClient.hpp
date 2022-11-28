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

#include <future>
#include <memory>

#include "SimulationCreationRequestDto.hpp"
#include "SimulationCreationResponseDto.hpp"
#include "SystemStatusDto.hpp"
#include "ParticipantStatusDto.hpp"
#include "ServiceDto.hpp"
#include "DataPublisherDto.hpp"
#include "RpcClientDto.hpp"
#include "SimulationEndDto.hpp"

namespace SilKit {
namespace Dashboard {

class IDashboardSystemServiceClient
{
public:
    virtual ~IDashboardSystemServiceClient() = default;

    virtual std::future<oatpp::Object<SimulationCreationResponseDto>> CreateSimulation(
        oatpp::Object<SimulationCreationRequestDto> simulation) = 0;

    virtual void AddParticipantToSimulation(oatpp::UInt32 simulationId, oatpp::String participantName) = 0;

    virtual void AddParticipantStatusForSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                   oatpp::Object<ParticipantStatusDto> participantStatus) = 0;

    virtual void AddCanControllerForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                            oatpp::String canonicalName,
                                                            oatpp::Object<ServiceDto> canController) = 0;

    virtual void AddEthernetControllerForParticipantOfSimulation(oatpp::UInt32 simulationId,
                                                                 oatpp::String participantName,
                                                                 oatpp::String canonicalName,
                                                                 oatpp::Object<ServiceDto> ethernetController) = 0;

    virtual void AddFlexrayControllerForParticipantOfSimulation(oatpp::UInt32 simulationId,
                                                                oatpp::String participantName,
                                                                oatpp::String canonicalName,
                                                                oatpp::Object<ServiceDto> flexrayController) = 0;

    virtual void AddLinControllerForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                            oatpp::String canonicalName,
                                                            oatpp::Object<ServiceDto> linController) = 0;

    virtual void AddDataPublisherForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                            oatpp::String canonicalName,
                                                            oatpp::Object<DataPublisherDto> dataPublisher) = 0;

    virtual void AddDataSubscriberForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                             oatpp::String canonicalName,
                                                             oatpp::Object<ServiceDto> dataSubscriber) = 0;

    virtual void AddRpcClientForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                        oatpp::String canonicalName,
                                                        oatpp::Object<RpcClientDto> rpcClient) = 0;

    virtual void AddRpcServerForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                        oatpp::String canonicalName,
                                                        oatpp::Object<ServiceDto> rpcServer) = 0;

    virtual void AddCanNetworkToSimulation(oatpp::UInt32 simulationId, oatpp::String networkName) = 0;

    virtual void AddEthernetNetworkToSimulation(oatpp::UInt32 simulationId, oatpp::String networkName) = 0;

    virtual void AddFlexrayNetworkToSimulation(oatpp::UInt32 simulationId, oatpp::String networkName) = 0;

    virtual void AddLinNetworkToSimulation(oatpp::UInt32 simulationId, oatpp::String networkName) = 0;

    virtual void UpdateSystemStatusForSimulation(oatpp::UInt32 simulationId,
                                                 oatpp::Object<SystemStatusDto> systemStatus) = 0;

    virtual void SetSimulationEnd(oatpp::UInt32 simulationId, oatpp::Object<SimulationEndDto> simulation) = 0;
};

} // namespace Dashboard
} // namespace SilKit

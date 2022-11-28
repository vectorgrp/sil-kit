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

#include "IDashboardSystemServiceClient.hpp"

#include "oatpp/core/async/Executor.hpp"

#include "silkit/services/logging/ILogger.hpp"

#include "DashboardSystemApiClient.hpp"

namespace SilKit {
namespace Dashboard {

class DashboardSystemServiceClient : public IDashboardSystemServiceClient
{
public:
    DashboardSystemServiceClient(Services::Logging::ILogger* logger,
                                 std::shared_ptr<DashboardSystemApiClient> dashboardSystemApiClient,
                                 std::shared_ptr<oatpp::data::mapping::ObjectMapper> objectMapper,
                                 std::shared_ptr<oatpp::async::Executor> executor);
    ~DashboardSystemServiceClient();

public:
    std::future<oatpp::Object<SimulationCreationResponseDto>> CreateSimulation(
        oatpp::Object<SimulationCreationRequestDto> simulation) override;

    void AddParticipantToSimulation(oatpp::UInt32 simulationId, oatpp::String participantName) override;

    void AddParticipantStatusForSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                           oatpp::Object<ParticipantStatusDto> participantStatus) override;

    void AddCanControllerForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                    oatpp::String canonicalName,
                                                    oatpp::Object<ServiceDto> canController) override;

    void AddEthernetControllerForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                         oatpp::String canonicalName,
                                                         oatpp::Object<ServiceDto> ethernetController) override;

    void AddFlexrayControllerForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                        oatpp::String canonicalName,
                                                        oatpp::Object<ServiceDto> flexrayController) override;

    void AddLinControllerForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                    oatpp::String canonicalName,
                                                    oatpp::Object<ServiceDto> linController) override;

    void AddDataPublisherForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                    oatpp::String canonicalName,
                                                    oatpp::Object<DataPublisherDto> dataPublisher) override;

    void AddDataSubscriberForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                     oatpp::String canonicalName,
                                                     oatpp::Object<ServiceDto> dataSubscriber) override;

    void AddRpcClientForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                oatpp::String canonicalName,
                                                oatpp::Object<RpcClientDto> rpcClient) override;

    void AddRpcServerForParticipantOfSimulation(oatpp::UInt32 simulationId, oatpp::String participantName,
                                                oatpp::String canonicalName,
                                                oatpp::Object<ServiceDto> rpcServer) override;

    void AddCanNetworkToSimulation(oatpp::UInt32 simulationId, oatpp::String networkName) override;

    void AddEthernetNetworkToSimulation(oatpp::UInt32 simulationId, oatpp::String networkName) override;

    void AddFlexrayNetworkToSimulation(oatpp::UInt32 simulationId, oatpp::String networkName) override;

    void AddLinNetworkToSimulation(oatpp::UInt32 simulationId, oatpp::String networkName) override;

    void UpdateSystemStatusForSimulation(oatpp::UInt32 simulationId,
                                         oatpp::Object<SystemStatusDto> systemStatus) override;

    void SetSimulationEnd(oatpp::UInt32 simulationId, oatpp::Object<SimulationEndDto> simulation) override;

private:
    void OnSimulationCreationResponseBody(oatpp::Object<SimulationCreationResponseDto> simulation);

private:
    std::promise<oatpp::Object<SimulationCreationResponseDto>> _simulationCreationPromise;
    Services::Logging::ILogger* _logger;
    std::shared_ptr<DashboardSystemApiClient> _dashboardSystemApiClient;
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> _objectMapper;
    std::shared_ptr<oatpp::async::Executor> _executor;
};

} // namespace Dashboard
} // namespace SilKit

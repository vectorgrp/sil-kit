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

#include <memory>

#include "silkit/services/logging/ILogger.hpp"

#include "DashboardSystemApiClient.hpp"

namespace SilKit {
namespace Dashboard {

class DashboardSystemServiceClient : public IDashboardSystemServiceClient
{
public:
    DashboardSystemServiceClient(Services::Logging::ILogger* logger,
                                 std::shared_ptr<DashboardSystemApiClient> dashboardSystemApiClient,
                                 std::shared_ptr<oatpp::data::mapping::ObjectMapper> objectMapper);
    ~DashboardSystemServiceClient();

public:
    oatpp::Object<SimulationCreationResponseDto> CreateSimulation(
        oatpp::Object<SimulationCreationRequestDto> simulation) override;

    void UpdateSimulation(oatpp::UInt64 simulationId, oatpp::Object<BulkSimulationDto> bulkSimulation) override;

    void AddParticipantToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName) override;

    void AddParticipantStatusForSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                           oatpp::Object<ParticipantStatusDto> participantStatus) override;

    void AddCanControllerForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                    oatpp::UInt64 serviceId,
                                                    oatpp::Object<ServiceDto> canController) override;

    void AddEthernetControllerForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                         oatpp::UInt64 serviceId,
                                                         oatpp::Object<ServiceDto> ethernetController) override;

    void AddFlexrayControllerForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                        oatpp::UInt64 serviceId,
                                                        oatpp::Object<ServiceDto> flexrayController) override;

    void AddLinControllerForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                    oatpp::UInt64 serviceId,
                                                    oatpp::Object<ServiceDto> linController) override;

    void AddDataPublisherForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                    oatpp::UInt64 serviceId,
                                                    oatpp::Object<DataPublisherDto> dataPublisher) override;

    void AddDataSubscriberForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                     oatpp::UInt64 serviceId,
                                                     oatpp::Object<DataSubscriberDto> dataSubscriber) override;

    void AddDataSubscriberInternalForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                             oatpp::String parentServiceId, oatpp::UInt64 serviceId,
                                                             oatpp::Object<ServiceDto> dataSubscriberInternal) override;

    void AddRpcClientForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                oatpp::UInt64 serviceId,
                                                oatpp::Object<RpcClientDto> rpcClient) override;

    void AddRpcServerForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                oatpp::UInt64 serviceId,
                                                oatpp::Object<RpcServerDto> rpcServer) override;

    void AddRpcServerInternalForParticipantOfSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                        oatpp::String parentServiceId, oatpp::UInt64 serviceId,
                                                        oatpp::Object<ServiceDto> rpcServerInternal) override;

    void AddCanNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                   oatpp::String networkName) override;

    void AddEthernetNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                        oatpp::String networkName) override;

    void AddFlexrayNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                       oatpp::String networkName) override;

    void AddLinNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                   oatpp::String networkName) override;

    void UpdateSystemStatusForSimulation(oatpp::UInt64 simulationId,
                                         oatpp::Object<SystemStatusDto> systemStatus) override;

    void SetSimulationEnd(oatpp::UInt64 simulationId, oatpp::Object<SimulationEndDto> simulation) override;

private:
    void Log(std::shared_ptr<oatpp::web::client::RequestExecutor::Response> response, const std::string& message);

private:
    Services::Logging::ILogger* _logger;
    std::shared_ptr<DashboardSystemApiClient> _dashboardSystemApiClient;
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> _objectMapper;
};

} // namespace Dashboard
} // namespace SilKit

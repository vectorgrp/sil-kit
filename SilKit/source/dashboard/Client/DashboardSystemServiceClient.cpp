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

#include "DashboardSystemServiceClient.hpp"

#include "LoggerMessage.hpp"

#include OATPP_CODEGEN_BEGIN(ApiClient)

using namespace std::chrono_literals;

namespace SilKit {
namespace Dashboard {

DashboardSystemServiceClient::DashboardSystemServiceClient(
    Services::Logging::ILogger* logger, std::shared_ptr<DashboardSystemApiClient> dashboardSystemApiClient,
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> objectMapper)
    : _logger(logger)
    , _dashboardSystemApiClient(dashboardSystemApiClient)
    , _objectMapper(objectMapper)
{
}

DashboardSystemServiceClient::~DashboardSystemServiceClient() {}

void DashboardSystemServiceClient::UpdateSimulation(oatpp::UInt64 simulationId,
                                                    oatpp::Object<BulkSimulationDto> bulkSimulation)
{
    auto response = _dashboardSystemApiClient->updateSimulation(simulationId, bulkSimulation);
    Log(response, "updating simulation");
}

oatpp::Object<SimulationCreationResponseDto> DashboardSystemServiceClient::CreateSimulation(
    oatpp::Object<SimulationCreationRequestDto> simulation)
{
    auto response = _dashboardSystemApiClient->createSimulation(simulation);
    Log(response, "creating simulation");
    if (response && response->getStatusCode() == 201)
    {
        return response->readBodyToDto<oatpp::Object<SimulationCreationResponseDto>>(_objectMapper);
    }
    return nullptr;
}

void DashboardSystemServiceClient::AddParticipantToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName)
{
    auto response = _dashboardSystemApiClient->addParticipantToSimulation(simulationId, participantName);
    Log(response, "adding participant");
}

void DashboardSystemServiceClient::AddParticipantStatusForSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::Object<ParticipantStatusDto> participantStatus)
{
    auto response =
        _dashboardSystemApiClient->addParticipantStatusForSimulation(simulationId, participantName, participantStatus);
    Log(response, "adding participant status");
}

void DashboardSystemServiceClient::AddCanControllerForParticipantOfSimulation(oatpp::UInt64 simulationId,
                                                                              oatpp::String participantName,
                                                                              oatpp::UInt64 serviceId,
                                                                              oatpp::Object<ServiceDto> canController)
{
    auto response = _dashboardSystemApiClient->addCanControllerForParticipantOfSimulation(simulationId, participantName,
                                                                                          serviceId, canController);
    Log(response, "adding can controller");
}

void DashboardSystemServiceClient::AddEthernetControllerForParticipantOfSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::UInt64 serviceId,
    oatpp::Object<ServiceDto> ethernetController)
{
    auto response = _dashboardSystemApiClient->addEthernetControllerForParticipantOfSimulation(
        simulationId, participantName, serviceId, ethernetController);
    Log(response, "adding ethernet controller");
}

void DashboardSystemServiceClient::AddFlexrayControllerForParticipantOfSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::UInt64 serviceId,
    oatpp::Object<ServiceDto> flexrayController)
{
    auto response = _dashboardSystemApiClient->addFlexrayControllerForParticipantOfSimulation(
        simulationId, participantName, serviceId, flexrayController);
    Log(response, "adding flexray controller");
}

void DashboardSystemServiceClient::AddLinControllerForParticipantOfSimulation(oatpp::UInt64 simulationId,
                                                                              oatpp::String participantName,
                                                                              oatpp::UInt64 serviceId,
                                                                              oatpp::Object<ServiceDto> linController)
{
    auto response = _dashboardSystemApiClient->addLinControllerForParticipantOfSimulation(simulationId, participantName,
                                                                                          serviceId, linController);
    Log(response, "adding lin controller");
}

void DashboardSystemServiceClient::AddDataPublisherForParticipantOfSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::UInt64 serviceId,
    oatpp::Object<DataPublisherDto> dataPublisher)
{
    auto response = _dashboardSystemApiClient->addDataPublisherForParticipantOfSimulation(simulationId, participantName,
                                                                                          serviceId, dataPublisher);
    Log(response, "adding data publisher");
}

void DashboardSystemServiceClient::AddDataSubscriberForParticipantOfSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::UInt64 serviceId,
    oatpp::Object<DataSubscriberDto> dataSubscriber)
{
    auto response = _dashboardSystemApiClient->addDataSubscriberForParticipantOfSimulation(
        simulationId, participantName, serviceId, dataSubscriber);
    Log(response, "adding data subscriber");
}

void DashboardSystemServiceClient::AddDataSubscriberInternalForParticipantOfSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::String parentServiceId, oatpp::UInt64 serviceId,
    oatpp::Object<ServiceDto> dataSubscriberInternal)
{
    auto response = _dashboardSystemApiClient->addDataSubscriberInternalForParticipantOfSimulation(
        simulationId, participantName, parentServiceId, serviceId, dataSubscriberInternal);
    Log(response, "adding data subscriber internal");
}

void DashboardSystemServiceClient::AddRpcClientForParticipantOfSimulation(oatpp::UInt64 simulationId,
                                                                          oatpp::String participantName,
                                                                          oatpp::UInt64 serviceId,
                                                                          oatpp::Object<RpcClientDto> rpcClient)
{
    auto response = _dashboardSystemApiClient->addRpcClientForParticipantOfSimulation(simulationId, participantName,
                                                                                      serviceId, rpcClient);
    Log(response, "adding rpc client");
}

void DashboardSystemServiceClient::AddRpcServerForParticipantOfSimulation(oatpp::UInt64 simulationId,
                                                                          oatpp::String participantName,
                                                                          oatpp::UInt64 serviceId,
                                                                          oatpp::Object<RpcServerDto> rpcServer)
{
    auto response = _dashboardSystemApiClient->addRpcServerForParticipantOfSimulation(simulationId, participantName,
                                                                                      serviceId, rpcServer);
    Services::Logging::Debug(_logger, "Dashboard: adding rpc server returned {}", response->getStatusCode());
    Log(response, "adding rpc server");
}

void DashboardSystemServiceClient::AddRpcServerInternalForParticipantOfSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::String parentServiceId, oatpp::UInt64 serviceId,
    oatpp::Object<ServiceDto> rpcServerInternal)
{
    auto response = _dashboardSystemApiClient->addRpcServerInternalForParticipantOfSimulation(
        simulationId, participantName, parentServiceId, serviceId, rpcServerInternal);
    Log(response, "adding rpc server internal");
}

void DashboardSystemServiceClient::AddCanNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                             oatpp::String networkName)
{
    auto response = _dashboardSystemApiClient->addCanNetworkToSimulation(simulationId, participantName, networkName);
    Log(response, "adding can network");
}

void DashboardSystemServiceClient::AddEthernetNetworkToSimulation(oatpp::UInt64 simulationId,
                                                                  oatpp::String participantName,
                                                                  oatpp::String networkName)
{
    auto response =
        _dashboardSystemApiClient->addEthernetNetworkToSimulation(simulationId, participantName, networkName);
    Log(response, "adding ethernet network");
}

void DashboardSystemServiceClient::AddFlexrayNetworkToSimulation(oatpp::UInt64 simulationId,
                                                                 oatpp::String participantName,
                                                                 oatpp::String networkName)
{
    auto response =
        _dashboardSystemApiClient->addFlexrayNetworkToSimulation(simulationId, participantName, networkName);
    Log(response, "adding flexray network");
}

void DashboardSystemServiceClient::AddLinNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName,
                                                             oatpp::String networkName)
{
    auto response = _dashboardSystemApiClient->addLinNetworkToSimulation(simulationId, participantName, networkName);
    Log(response, "adding lin network");
}

void DashboardSystemServiceClient::UpdateSystemStatusForSimulation(oatpp::UInt64 simulationId,
                                                                   oatpp::Object<SystemStatusDto> systemStatus)
{
    auto response = _dashboardSystemApiClient->updateSystemStatusForSimulation(simulationId, systemStatus);
    Log(response, "updating system status");
}

void DashboardSystemServiceClient::SetSimulationEnd(oatpp::UInt64 simulationId,
                                                    oatpp::Object<SimulationEndDto> simulation)
{
    auto response = _dashboardSystemApiClient->setSimulationEnd(simulationId, simulation);
    Log(response, "setting simulation end");
}

void DashboardSystemServiceClient::Log(std::shared_ptr<oatpp::web::client::RequestExecutor::Response> response,
                                       const std::string& message)
{
    if (!response)
    {
        Services::Logging::Error(_logger, "Dashboard: {} server unavailable", message);
    }
    else if (response->getStatusCode() >= 400)
    {
        Services::Logging::Error(_logger, "Dashboard: {} returned {}", message, response->getStatusCode());
    }
    else
    {
        Services::Logging::Debug(_logger, "Dashboard: {} returned {}", message, response->getStatusCode());
    }
}

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(ApiClient)

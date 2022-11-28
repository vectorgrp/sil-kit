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

#include "CreateSimulationCoroutine.hpp"
#include "SendCoroutine.hpp"

#include OATPP_CODEGEN_BEGIN(ApiClient)

namespace SilKit {
namespace Dashboard {

DashboardSystemServiceClient::DashboardSystemServiceClient(
    Services::Logging::ILogger* logger, std::shared_ptr<DashboardSystemApiClient> dashboardSystemApiClient,
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> objectMapper, std::shared_ptr<oatpp::async::Executor> executor)
    : _logger(logger)
    , _dashboardSystemApiClient(dashboardSystemApiClient)
    , _objectMapper(objectMapper)
    , _executor(executor)
{
}

DashboardSystemServiceClient::~DashboardSystemServiceClient()
{
}

std::future<oatpp::Object<SimulationCreationResponseDto>> DashboardSystemServiceClient::CreateSimulation(
    oatpp::Object<SimulationCreationRequestDto> simulation)
{
    _executor->execute<CreateSimulationCoroutine>(
        _logger, 
        _objectMapper,
        std::bind(&DashboardSystemApiClient::createSimulation, _dashboardSystemApiClient, simulation, nullptr),
        std::bind(&DashboardSystemServiceClient::OnSimulationCreationResponseBody, this, std::placeholders::_1));
    return _simulationCreationPromise.get_future();
}

void DashboardSystemServiceClient::OnSimulationCreationResponseBody(
    oatpp::Object<SimulationCreationResponseDto> simulation)
{
    _simulationCreationPromise.set_value(simulation);
}

void DashboardSystemServiceClient::AddParticipantToSimulation(oatpp::UInt32 simulationId, oatpp::String participantName)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::addParticipantToSimulation,
                                                _dashboardSystemApiClient, simulationId, participantName, nullptr),
                                      "adding participant");
}

void DashboardSystemServiceClient::AddParticipantStatusForSimulation(
    oatpp::UInt32 simulationId, oatpp::String participantName, oatpp::Object<ParticipantStatusDto> participantStatus)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addParticipantStatusForSimulation, _dashboardSystemApiClient, simulationId,
                  participantName, participantStatus, nullptr),
        "adding participant status");
}

void DashboardSystemServiceClient::AddCanControllerForParticipantOfSimulation(oatpp::UInt32 simulationId,
                                                                              oatpp::String participantName,
                                                                              oatpp::String canonicalName,
                                                                              oatpp::Object<ServiceDto> canController)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addCanControllerForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, canonicalName, canController, nullptr),
        "adding can controller");
}

void DashboardSystemServiceClient::AddEthernetControllerForParticipantOfSimulation(
    oatpp::UInt32 simulationId, oatpp::String participantName, oatpp::String canonicalName,
    oatpp::Object<ServiceDto> ethernetController)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addEthernetControllerForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, canonicalName, ethernetController, nullptr),
        "adding ethernet controller");
}

void DashboardSystemServiceClient::AddFlexrayControllerForParticipantOfSimulation(
    oatpp::UInt32 simulationId, oatpp::String participantName, oatpp::String canonicalName,
    oatpp::Object<ServiceDto> flexrayController)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addFlexrayControllerForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, canonicalName, flexrayController, nullptr),
        "adding flexray controller");
}

void DashboardSystemServiceClient::AddLinControllerForParticipantOfSimulation(oatpp::UInt32 simulationId,
                                                                              oatpp::String participantName,
                                                                              oatpp::String canonicalName,
                                                                              oatpp::Object<ServiceDto> linController)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addLinControllerForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, canonicalName, linController, nullptr),
        "adding lin controller");
}

void DashboardSystemServiceClient::AddDataPublisherForParticipantOfSimulation(
    oatpp::UInt32 simulationId, oatpp::String participantName, oatpp::String canonicalName,
    oatpp::Object<DataPublisherDto> dataPublisher)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addDataPublisherForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, canonicalName, dataPublisher, nullptr),
        "adding data publisher");
}

void DashboardSystemServiceClient::AddDataSubscriberForParticipantOfSimulation(oatpp::UInt32 simulationId,
                                                                               oatpp::String participantName,
                                                                               oatpp::String canonicalName,
                                                                               oatpp::Object<ServiceDto> dataSubscriber)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addDataSubscriberForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, canonicalName, dataSubscriber, nullptr),
        "adding data subscriber");
}

void DashboardSystemServiceClient::AddRpcClientForParticipantOfSimulation(oatpp::UInt32 simulationId,
                                                                          oatpp::String participantName,
                                                                          oatpp::String canonicalName,
                                                                          oatpp::Object<RpcClientDto> rpcClient)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addRpcClientForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, canonicalName, rpcClient, nullptr),
        "adding rpc client");
}

void DashboardSystemServiceClient::AddRpcServerForParticipantOfSimulation(oatpp::UInt32 simulationId,
                                                                          oatpp::String participantName,
                                                                          oatpp::String canonicalName,
                                                                          oatpp::Object<ServiceDto> rpcServer)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addRpcServerForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, canonicalName, rpcServer, nullptr),
        "adding rpc server");
}

void DashboardSystemServiceClient::AddCanNetworkToSimulation(oatpp::UInt32 simulationId, oatpp::String networkName)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::addCanNetworkToSimulation,
                                                _dashboardSystemApiClient, simulationId, networkName, nullptr),
                                      "adding can network");
}

void DashboardSystemServiceClient::AddEthernetNetworkToSimulation(oatpp::UInt32 simulationId, oatpp::String networkName)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::addEthernetNetworkToSimulation,
                                                _dashboardSystemApiClient, simulationId, networkName, nullptr),
                                      "adding ethernet network");
}

void DashboardSystemServiceClient::AddFlexrayNetworkToSimulation(oatpp::UInt32 simulationId, oatpp::String networkName)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::addFlexrayNetworkToSimulation,
                                                _dashboardSystemApiClient, simulationId, networkName, nullptr),
                                      "adding flexray network");
}

void DashboardSystemServiceClient::AddLinNetworkToSimulation(oatpp::UInt32 simulationId, oatpp::String networkName)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::addLinNetworkToSimulation,
                                                _dashboardSystemApiClient, simulationId, networkName, nullptr),
                                      "adding lin network");
}

void DashboardSystemServiceClient::UpdateSystemStatusForSimulation(oatpp::UInt32 simulationId,
                                                                   oatpp::Object<SystemStatusDto> systemStatus)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::updateSystemStatusForSimulation,
                                                _dashboardSystemApiClient, simulationId, systemStatus, nullptr),
                                      "updating system status");
}

void DashboardSystemServiceClient::SetSimulationEnd(oatpp::UInt32 simulationId,
                                                    oatpp::Object<SimulationEndDto> simulation)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::setSimulationEnd, _dashboardSystemApiClient,
                                                simulationId, simulation, nullptr),
                                      "setting simulation end");
}

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(ApiClient)

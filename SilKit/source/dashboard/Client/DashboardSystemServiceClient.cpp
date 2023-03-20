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
        _logger, _objectMapper,
        std::bind(&DashboardSystemApiClient::createSimulation, _dashboardSystemApiClient, simulation, nullptr),
        std::bind(&DashboardSystemServiceClient::OnSimulationCreationResponseBody, this, std::placeholders::_1));
    return _simulationCreationPromise.get_future();
}

void DashboardSystemServiceClient::OnSimulationCreationResponseBody(
    oatpp::Object<SimulationCreationResponseDto> simulation)
{
    _simulationCreationPromise.set_value(simulation);
}

void DashboardSystemServiceClient::AddParticipantToSimulation(oatpp::UInt64 simulationId, oatpp::String participantName)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::addParticipantToSimulation,
                                                _dashboardSystemApiClient, simulationId, participantName, nullptr),
                                      "adding participant");
}

void DashboardSystemServiceClient::AddParticipantStatusForSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::Object<ParticipantStatusDto> participantStatus)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addParticipantStatusForSimulation, _dashboardSystemApiClient, simulationId,
                  participantName, participantStatus, nullptr),
        "adding participant status");
}

void DashboardSystemServiceClient::AddCanControllerForParticipantOfSimulation(oatpp::UInt64 simulationId,
                                                                              oatpp::String participantName,
                                                                              oatpp::UInt64 serviceId,
                                                                              oatpp::Object<ServiceDto> canController)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addCanControllerForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, serviceId, canController, nullptr),
        "adding can controller");
}

void DashboardSystemServiceClient::AddEthernetControllerForParticipantOfSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::UInt64 serviceId,
    oatpp::Object<ServiceDto> ethernetController)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addEthernetControllerForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, serviceId, ethernetController, nullptr),
        "adding ethernet controller");
}

void DashboardSystemServiceClient::AddFlexrayControllerForParticipantOfSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::UInt64 serviceId,
    oatpp::Object<ServiceDto> flexrayController)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addFlexrayControllerForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, serviceId, flexrayController, nullptr),
        "adding flexray controller");
}

void DashboardSystemServiceClient::AddLinControllerForParticipantOfSimulation(oatpp::UInt64 simulationId,
                                                                              oatpp::String participantName,
                                                                              oatpp::UInt64 serviceId,
                                                                              oatpp::Object<ServiceDto> linController)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addLinControllerForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, serviceId, linController, nullptr),
        "adding lin controller");
}

void DashboardSystemServiceClient::AddDataPublisherForParticipantOfSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::UInt64 serviceId,
    oatpp::Object<DataPublisherDto> dataPublisher)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addDataPublisherForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, serviceId, dataPublisher, nullptr),
        "adding data publisher");
}

void DashboardSystemServiceClient::AddDataSubscriberForParticipantOfSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::UInt64 serviceId,
    oatpp::Object<DataSubscriberDto> dataSubscriber)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addDataSubscriberForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, serviceId, dataSubscriber, nullptr),
        "adding data subscriber");
}

void DashboardSystemServiceClient::AddDataSubscriberInternalForParticipantOfSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::String parentServiceId, oatpp::UInt64 serviceId,
    oatpp::Object<ServiceDto> dataSubscriberInternal)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addDataSubscriberInternalForParticipantOfSimulation,
                  _dashboardSystemApiClient, simulationId, participantName, parentServiceId, serviceId,
                  dataSubscriberInternal, nullptr),
        "adding data subscriber internal");
}

void DashboardSystemServiceClient::AddRpcClientForParticipantOfSimulation(oatpp::UInt64 simulationId,
                                                                          oatpp::String participantName,
                                                                          oatpp::UInt64 serviceId,
                                                                          oatpp::Object<RpcClientDto> rpcClient)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addRpcClientForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, serviceId, rpcClient, nullptr),
        "adding rpc client");
}

void DashboardSystemServiceClient::AddRpcServerForParticipantOfSimulation(oatpp::UInt64 simulationId,
                                                                          oatpp::String participantName,
                                                                          oatpp::UInt64 serviceId,
                                                                          oatpp::Object<RpcServerDto> rpcServer)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addRpcServerForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, serviceId, rpcServer, nullptr),
        "adding rpc server");
}

void DashboardSystemServiceClient::AddRpcServerInternalForParticipantOfSimulation(
    oatpp::UInt64 simulationId, oatpp::String participantName, oatpp::String parentServiceId, oatpp::UInt64 serviceId,
    oatpp::Object<ServiceDto> rpcServerInternal)
{
    _executor->execute<SendCoroutine>(
        _logger,
        std::bind(&DashboardSystemApiClient::addRpcServerInternalForParticipantOfSimulation, _dashboardSystemApiClient,
                  simulationId, participantName, parentServiceId, serviceId, rpcServerInternal, nullptr),
        "adding rpc server internal");
}

void DashboardSystemServiceClient::AddCanNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String networkName)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::addCanNetworkToSimulation,
                                                _dashboardSystemApiClient, simulationId, networkName, nullptr),
                                      "adding can network");
}

void DashboardSystemServiceClient::AddEthernetNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String networkName)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::addEthernetNetworkToSimulation,
                                                _dashboardSystemApiClient, simulationId, networkName, nullptr),
                                      "adding ethernet network");
}

void DashboardSystemServiceClient::AddFlexrayNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String networkName)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::addFlexrayNetworkToSimulation,
                                                _dashboardSystemApiClient, simulationId, networkName, nullptr),
                                      "adding flexray network");
}

void DashboardSystemServiceClient::AddLinNetworkToSimulation(oatpp::UInt64 simulationId, oatpp::String networkName)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::addLinNetworkToSimulation,
                                                _dashboardSystemApiClient, simulationId, networkName, nullptr),
                                      "adding lin network");
}

void DashboardSystemServiceClient::UpdateSystemStatusForSimulation(oatpp::UInt64 simulationId,
                                                                   oatpp::Object<SystemStatusDto> systemStatus)
{
    _executor->execute<SendCoroutine>(_logger,
                                      std::bind(&DashboardSystemApiClient::updateSystemStatusForSimulation,
                                                _dashboardSystemApiClient, simulationId, systemStatus, nullptr),
                                      "updating system status");
}

void DashboardSystemServiceClient::SetSimulationEnd(oatpp::UInt64 simulationId,
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

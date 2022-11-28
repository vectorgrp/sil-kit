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

#include "SilKitEventHandler.hpp"

#include "ILogger.hpp"
#include "SetThreadName.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/SilKit.hpp"
#include "Uri.hpp"

namespace SilKit {
namespace Dashboard {

SilKitEventHandler::SilKitEventHandler(Services::Logging::ILogger* logger,
                                       std::shared_ptr<IDashboardSystemServiceClient> dashboardSystemServiceClient,
                                       std::shared_ptr<ISilKitToOatppMapper> silKitToOatppMapper)
    : _logger(logger)
    , _dashboardSystemServiceClient(dashboardSystemServiceClient)
    , _silKitToOatppMapper(silKitToOatppMapper)
{
}

SilKitEventHandler::~SilKitEventHandler()
{
    _stopping = true;
    if (_simulationCreationThread.joinable())
    {
        _simulationCreationThread.join();
    }
}

std::future<bool> SilKitEventHandler::OnStart(const std::string& connectUri, uint64_t time)
{
    _logger->Info("Dashboard: creating simulation");
    _simulationCreationThread = std::thread{[this, &connectUri, time]() {
        SilKit::Util::SetThreadName("SK-Dash-Evt");
        Run(connectUri, time);
    }};
    return _simulationCreatedPromise.get_future();
}

void SilKitEventHandler::Run(const std::string& connectUri, uint64_t time)
{
    auto simulationCreated = _dashboardSystemServiceClient->CreateSimulation(
        _silKitToOatppMapper->CreateSimulationCreationRequestDto(connectUri, time));
    std::future_status simulationCreatedStatus;
    do
    {
        simulationCreatedStatus = simulationCreated.wait_for(std::chrono::seconds{1});
    } while (!_stopping && simulationCreatedStatus != std::future_status::ready);
    if (simulationCreatedStatus == std::future_status::ready)
    {
        auto simulation = simulationCreated.get();
        if (simulation)
        {
            Services::Logging::Info(_logger , "Dashboard: created simulation with id {}", simulation->id);
            _simulationId = simulation->id;
        }
        _simulationCreatedPromise.set_value(simulation != nullptr);
        return;
    }
    _logger->Warn("Dashboard: creating simulation: giving up...");
    _simulationCreatedPromise.set_value(false);
}

void SilKitEventHandler::OnShutdown(uint64_t time)
{
    Services::Logging::Info(_logger, "Dashboard: setting end for simulation {}", _simulationId);
    _dashboardSystemServiceClient->SetSimulationEnd(_simulationId.load(),
                                                    _silKitToOatppMapper->CreateSimulationEndDto(time));
}

void SilKitEventHandler::OnParticipantConnected(
    const Services::Orchestration::ParticipantConnectionInformation& participantInformation)
{
    Services::Logging::Debug(_logger, "Dashboard: adding participant for simulation {} {}", _simulationId,
                             participantInformation.participantName);
    auto participantName = SilKit::Core::Uri::UrlEncode(participantInformation.participantName);
    _dashboardSystemServiceClient->AddParticipantToSimulation(_simulationId.load(), participantName);
}

void SilKitEventHandler::OnSystemStateChanged(Services::Orchestration::SystemState systemState)
{
    Services::Logging::Debug(_logger, "Dashboard: updating system state for simulation {} {}", _simulationId, systemState);
    _dashboardSystemServiceClient->UpdateSystemStatusForSimulation(
        _simulationId.load(), _silKitToOatppMapper->CreateSystemStatusDto(systemState));
}

void SilKitEventHandler::OnParticipantStatusChanged(const Services::Orchestration::ParticipantStatus& participantStatus)
{
    Services::Logging::Debug(_logger, "Dashboard: adding participant status for simulation {} {} {}", _simulationId,
                             participantStatus.participantName, participantStatus.state);
    auto participantName = SilKit::Core::Uri::UrlEncode(participantStatus.participantName);
    _dashboardSystemServiceClient->AddParticipantStatusForSimulation(
        _simulationId.load(), participantName, _silKitToOatppMapper->CreateParticipantStatusDto(participantStatus));
}

void SilKitEventHandler::OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                                 const Core::ServiceDescriptor& serviceDescriptor)
{
    switch (serviceDescriptor.GetServiceType())
    {
    case Core::ServiceType::Controller: OnControllerCreated(_simulationId, serviceDescriptor); break;
    case Core::ServiceType::Link: OnLinkCreated(_simulationId, serviceDescriptor); break;
    default: break;
    }
}

bool IsInternalController(const Core::ServiceDescriptor& serviceDescriptor)
{
    return (serviceDescriptor.GetNetworkType() == Config::NetworkType::Data
            || serviceDescriptor.GetNetworkType() == Config::NetworkType::RPC)
           && serviceDescriptor.GetNetworkName() == "default";
}

void SilKitEventHandler::OnControllerCreated(uint32_t simulationId, const Core::ServiceDescriptor& serviceDescriptor)
{
    if (IsInternalController(serviceDescriptor))
    {
        return;
    }
    Services::Logging::Debug(_logger, "Dashboard: adding service for simulation {} {}", simulationId, serviceDescriptor);
    std::string controllerType;
    if (!serviceDescriptor.GetSupplementalDataItem(Core::Discovery::controllerType, controllerType))
    {
        throw SilKitError{"Missing key" + Core::Discovery::controllerType + " in supplementalData"};
    }
    auto participantName = SilKit::Core::Uri::UrlEncode(serviceDescriptor.GetParticipantName());
    auto serviceName = SilKit::Core::Uri::UrlEncode(serviceDescriptor.GetServiceName());
    if (controllerType == Core::Discovery::controllerTypeCan)
    {
        _dashboardSystemServiceClient->AddCanControllerForParticipantOfSimulation(
            simulationId, participantName, serviceName, _silKitToOatppMapper->CreateServiceDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeEthernet)
    {
        _dashboardSystemServiceClient->AddEthernetControllerForParticipantOfSimulation(
            simulationId, participantName, serviceName, _silKitToOatppMapper->CreateServiceDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeFlexray)
    {
        _dashboardSystemServiceClient->AddFlexrayControllerForParticipantOfSimulation(
            simulationId, participantName, serviceName, _silKitToOatppMapper->CreateServiceDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeLin)
    {
        _dashboardSystemServiceClient->AddLinControllerForParticipantOfSimulation(
            simulationId, participantName, serviceName, _silKitToOatppMapper->CreateServiceDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeDataPublisher)
    {
        _dashboardSystemServiceClient->AddDataPublisherForParticipantOfSimulation(
            simulationId, participantName, serviceName,
            _silKitToOatppMapper->CreateDataPublisherDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeDataSubscriberInternal)
    {
        _dashboardSystemServiceClient->AddDataSubscriberForParticipantOfSimulation(
            simulationId, participantName, serviceName, _silKitToOatppMapper->CreateServiceDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeRpcServerInternal)
    {
        _dashboardSystemServiceClient->AddRpcServerForParticipantOfSimulation(
            simulationId, participantName, serviceName, _silKitToOatppMapper->CreateServiceDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeRpcClient)
    {
        _dashboardSystemServiceClient->AddRpcClientForParticipantOfSimulation(
            simulationId, participantName, serviceName, _silKitToOatppMapper->CreateRpcClientDto(serviceDescriptor));
    }
}

void SilKitEventHandler::OnLinkCreated(uint32_t simulationId, const Core::ServiceDescriptor& serviceDescriptor)
{
    Services::Logging::Debug(_logger, "Dashboard: adding network for simulation {} {}", simulationId, serviceDescriptor);
    auto networkName = SilKit::Core::Uri::UrlEncode(serviceDescriptor.GetNetworkName());
    switch (serviceDescriptor.GetNetworkType())
    {
    case Config::NetworkType::CAN:
        _dashboardSystemServiceClient->AddCanNetworkToSimulation(simulationId, networkName);
        break;
    case Config::NetworkType::Ethernet:
        _dashboardSystemServiceClient->AddEthernetNetworkToSimulation(simulationId, networkName);
        break;
    case Config::NetworkType::FlexRay:
        _dashboardSystemServiceClient->AddFlexrayNetworkToSimulation(simulationId, networkName);
        break;
    case Config::NetworkType::LIN:
        _dashboardSystemServiceClient->AddLinNetworkToSimulation(simulationId, networkName);
        break;
    default: return;
    }
}

} // namespace Dashboard
} // namespace SilKit

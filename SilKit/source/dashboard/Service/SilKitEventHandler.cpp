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

#include "LoggerMessage.hpp"
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

SilKitEventHandler::~SilKitEventHandler() {}

uint64_t SilKitEventHandler::OnSimulationStart(const std::string& connectUri, uint64_t time)
{
    Services::Logging::Info(_logger, "Dashboard: creating simulation {} {}", connectUri, time);
    auto simulation = _dashboardSystemServiceClient->CreateSimulation(
        _silKitToOatppMapper->CreateSimulationCreationRequestDto(connectUri, time));
    if (simulation)
    {
        Services::Logging::Info(_logger, "Dashboard: created simulation with id {}", *simulation->id.get());
        return simulation->id;
    }
    _logger->Warn("Dashboard: creating simulation failed");
    return 0;
}

void SilKitEventHandler::OnSimulationEnd(uint64_t simulationId, uint64_t time)
{
    Services::Logging::Info(_logger, "Dashboard: setting end for simulation {}", simulationId);
    _dashboardSystemServiceClient->SetSimulationEnd(simulationId, _silKitToOatppMapper->CreateSimulationEndDto(time));
}

void SilKitEventHandler::OnParticipantConnected(
    uint64_t simulationId, const Services::Orchestration::ParticipantConnectionInformation& participantInformation)
{
    Services::Logging::Debug(_logger, "Dashboard: adding participant for simulation {} {}", simulationId,
                             participantInformation.participantName);
    auto participantName = SilKit::Core::Uri::UrlEncode(participantInformation.participantName);
    _dashboardSystemServiceClient->AddParticipantToSimulation(simulationId, participantName);
}

void SilKitEventHandler::OnSystemStateChanged(uint64_t simulationId, Services::Orchestration::SystemState systemState)
{
    Services::Logging::Debug(_logger, "Dashboard: updating system state for simulation {} {}", simulationId,
                             systemState);
    _dashboardSystemServiceClient->UpdateSystemStatusForSimulation(
        simulationId, _silKitToOatppMapper->CreateSystemStatusDto(systemState));
}

void SilKitEventHandler::OnParticipantStatusChanged(uint64_t simulationId,
                                                    const Services::Orchestration::ParticipantStatus& participantStatus)
{
    Services::Logging::Debug(_logger, "Dashboard: adding participant status for simulation {} {} {}", simulationId,
                             participantStatus.participantName, participantStatus.state);
    auto participantName = SilKit::Core::Uri::UrlEncode(participantStatus.participantName);
    _dashboardSystemServiceClient->AddParticipantStatusForSimulation(
        simulationId, participantName, _silKitToOatppMapper->CreateParticipantStatusDto(participantStatus));
}

void SilKitEventHandler::OnServiceDiscoveryEvent(uint64_t simulationId,
                                                 Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                                 const Core::ServiceDescriptor& serviceDescriptor)
{
    SILKIT_UNUSED_ARG(discoveryType);
    switch (serviceDescriptor.GetServiceType())
    {
    case Core::ServiceType::Controller:
        OnControllerCreated(simulationId, serviceDescriptor);
        break;
    case Core::ServiceType::Link:
        OnLinkCreated(simulationId, serviceDescriptor);
        break;
    default:
        break;
    }
}

void SilKitEventHandler::OnBulkUpdate(uint64_t simulationId, const DashboardBulkUpdate& bulkUpdate)
{
    _dashboardSystemServiceClient->UpdateSimulation(simulationId,
                                                    _silKitToOatppMapper->CreateBulkSimulationDto(bulkUpdate));
}

void SilKitEventHandler::OnControllerCreated(uint64_t simulationId, const Core::ServiceDescriptor& serviceDescriptor)
{
    Services::Logging::Debug(_logger, "Dashboard: adding service for simulation {} {}", simulationId,
                             serviceDescriptor);
    std::string controllerType;
    if (!serviceDescriptor.GetSupplementalDataItem(Core::Discovery::controllerType, controllerType))
    {
        throw SilKitError{"Missing key" + Core::Discovery::controllerType + " in supplementalData"};
    }
    auto participantName = SilKit::Core::Uri::UrlEncode(serviceDescriptor.GetParticipantName());
    if (controllerType == Core::Discovery::controllerTypeCan)
    {
        _dashboardSystemServiceClient->AddCanControllerForParticipantOfSimulation(
            simulationId, participantName, serviceDescriptor.GetServiceId(),
            _silKitToOatppMapper->CreateServiceDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeEthernet)
    {
        _dashboardSystemServiceClient->AddEthernetControllerForParticipantOfSimulation(
            simulationId, participantName, serviceDescriptor.GetServiceId(),
            _silKitToOatppMapper->CreateServiceDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeFlexray)
    {
        _dashboardSystemServiceClient->AddFlexrayControllerForParticipantOfSimulation(
            simulationId, participantName, serviceDescriptor.GetServiceId(),
            _silKitToOatppMapper->CreateServiceDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeLin)
    {
        _dashboardSystemServiceClient->AddLinControllerForParticipantOfSimulation(
            simulationId, participantName, serviceDescriptor.GetServiceId(),
            _silKitToOatppMapper->CreateServiceDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeDataPublisher)
    {
        _dashboardSystemServiceClient->AddDataPublisherForParticipantOfSimulation(
            simulationId, participantName, serviceDescriptor.GetServiceId(),
            _silKitToOatppMapper->CreateDataPublisherDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeDataSubscriber)
    {
        _dashboardSystemServiceClient->AddDataSubscriberForParticipantOfSimulation(
            simulationId, participantName, serviceDescriptor.GetServiceId(),
            _silKitToOatppMapper->CreateDataSubscriberDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeDataSubscriberInternal)
    {
        std::string parentServiceId;
        if (!serviceDescriptor.GetSupplementalDataItem(Core::Discovery::supplKeyDataSubscriberInternalParentServiceID,
                                                       parentServiceId))
        {
            throw SilKitError{"Missing key" + Core::Discovery::supplKeyDataSubscriberInternalParentServiceID
                              + " in supplementalData"};
        }
        _dashboardSystemServiceClient->AddDataSubscriberInternalForParticipantOfSimulation(
            simulationId, participantName, parentServiceId, serviceDescriptor.GetServiceId(),
            _silKitToOatppMapper->CreateServiceDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeRpcClient)
    {
        _dashboardSystemServiceClient->AddRpcClientForParticipantOfSimulation(
            simulationId, participantName, serviceDescriptor.GetServiceId(),
            _silKitToOatppMapper->CreateRpcClientDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeRpcServer)
    {
        _dashboardSystemServiceClient->AddRpcServerForParticipantOfSimulation(
            simulationId, participantName, serviceDescriptor.GetServiceId(),
            _silKitToOatppMapper->CreateRpcServerDto(serviceDescriptor));
    }
    else if (controllerType == Core::Discovery::controllerTypeRpcServerInternal)
    {
        std::string parentServiceId;
        if (!serviceDescriptor.GetSupplementalDataItem(Core::Discovery::supplKeyRpcServerInternalParentServiceID,
                                                       parentServiceId))
        {
            throw SilKitError{"Missing key" + Core::Discovery::supplKeyRpcServerInternalParentServiceID
                              + " in supplementalData"};
        }
        _dashboardSystemServiceClient->AddRpcServerInternalForParticipantOfSimulation(
            simulationId, participantName, parentServiceId, serviceDescriptor.GetServiceId(),
            _silKitToOatppMapper->CreateServiceDto(serviceDescriptor));
    }
}

void SilKitEventHandler::OnLinkCreated(uint64_t simulationId, const Core::ServiceDescriptor& serviceDescriptor)
{
    Services::Logging::Debug(_logger, "Dashboard: adding network for simulation {} {}", simulationId,
                             serviceDescriptor);
    auto participantName = SilKit::Core::Uri::UrlEncode(serviceDescriptor.GetParticipantName());
    auto networkName = SilKit::Core::Uri::UrlEncode(serviceDescriptor.GetNetworkName());
    switch (serviceDescriptor.GetNetworkType())
    {
    case Config::NetworkType::CAN:
        _dashboardSystemServiceClient->AddCanNetworkToSimulation(simulationId, participantName, networkName);
        break;
    case Config::NetworkType::Ethernet:
        _dashboardSystemServiceClient->AddEthernetNetworkToSimulation(simulationId, participantName, networkName);
        break;
    case Config::NetworkType::FlexRay:
        _dashboardSystemServiceClient->AddFlexrayNetworkToSimulation(simulationId, participantName, networkName);
        break;
    case Config::NetworkType::LIN:
        _dashboardSystemServiceClient->AddLinNetworkToSimulation(simulationId, participantName, networkName);
        break;
    default:
        return;
    }
}

} // namespace Dashboard
} // namespace SilKit

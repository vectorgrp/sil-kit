// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SimulatedNetworkRouter.hpp"
#include "IServiceDiscovery.hpp"
#include "NetworkSimulatorDatatypesInternal.hpp"
#include "Configuration.hpp"
#include "silkit/experimental/netsim/string_utils.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

SimulatedNetworkRouter::SimulatedNetworkRouter(Core::IParticipantInternal* participant, const std::string& networkName,
                                               SimulatedNetworkType networkType)
    : _participant{participant}
    , _networkName{networkName}
    , _networkType{networkType}
{
    // Here we register one ISimulator per actually simulated network to not register for unnecessary bus msg types
    // when simulating only one bus type.
    _participant->RegisterSimulator(this, _networkName, _networkType);

    _participant->AddAsyncSubscriptionsCompletionHandler([this]() {
        SilKit::Services::Logging::Debug(_participant->GetLogger(), "Announce simulation of network '{}' of type {}",
                                         _networkName, to_string(_networkType));
        // Announcing the network via ServiceDiscovery. Controllers on that network switch to simulated mode.
        AnnounceNetwork(_networkName, _networkType);
    });
}

void SimulatedNetworkRouter::AnnounceNetwork(const std::string& networkName, SimulatedNetworkType networkType)
{
    auto configNetworkType = ConvertNetworkTypeToConfig(networkType);

    std::string simulatorName = _participant->GetParticipantName();
    SilKit::Core::ServiceDescriptor linkDescriptor{};
    linkDescriptor.SetParticipantNameAndComputeId(simulatorName);
    linkDescriptor.SetNetworkType(configNetworkType);
    linkDescriptor.SetNetworkName(networkName);
    linkDescriptor.SetServiceName(networkName);
    // Controller switch to 'simulated' mode if they discover a ServiceType::Link with their networkName
    linkDescriptor.SetServiceType(SilKit::Core::ServiceType::Link);
    _participant->GetServiceDiscovery()->NotifyServiceCreated(std::move(linkDescriptor));
}

bool SimulatedNetworkRouter::AllowReception(const SilKit::Core::IServiceEndpoint* from)
{
    // Block messages from network simulation
    return from->GetServiceDescriptor().GetServiceType() != Core::ServiceType::SimulatedController;
}

void SimulatedNetworkRouter::AddSimulatedController(const std::string& fromParticipantName,
                                                    const std::string& controllerName, Core::EndpointId serviceId,
                                                    ControllerDescriptor controllerDescriptor,
                                                    ISimulatedController* userSimulatedController)
{
    _simulatedControllers[fromParticipantName].insert({serviceId, userSimulatedController});

    // Copy serviceDescriptor and overwrite with serviceId of receiving controller
    auto targetController = std::make_unique<TargetController>();
    targetController->participantName = fromParticipantName;
    auto fromCopy = Core::ServiceDescriptor(this->GetServiceDescriptor());
    fromCopy.SetServiceId(serviceId);
    fromCopy.SetServiceType(Core::ServiceType::SimulatedController);
    fromCopy.SetServiceName(controllerName);

    targetController->SetServiceDescriptor(std::move(fromCopy));
    _targetControllers.insert({controllerDescriptor, std::move(targetController)});
}

void SimulatedNetworkRouter::RemoveSimulatedController(const std::string& fromParticipantName,
                                                       Core::EndpointId serviceId,
                                                       ControllerDescriptor controllerDescriptor)
{
    // Remove from targetController map
    auto it_targetControllerByControllerDescriptor = _targetControllers.find(controllerDescriptor);
    if (it_targetControllerByControllerDescriptor != _targetControllers.end())
    {
        _targetControllers.erase(controllerDescriptor);
    }

    // Remove from lookup by participantName + serviceId
    auto it_simulatedControllersByParticipant = _simulatedControllers.find(fromParticipantName);
    if (it_simulatedControllersByParticipant != _simulatedControllers.end())
    {
        auto it_simulatedControllersByServiceId = it_simulatedControllersByParticipant->second.find(serviceId);
        if (it_simulatedControllersByServiceId != it_simulatedControllersByParticipant->second.end())
        {
            it_simulatedControllersByParticipant->second.erase(it_simulatedControllersByServiceId);
        }
        if (it_simulatedControllersByParticipant->second.empty())
        {
            _simulatedControllers.erase(it_simulatedControllersByParticipant);
        }
    }
}

auto SimulatedNetworkRouter::GetSimulatedControllerFromServiceEndpoint(const SilKit::Core::IServiceEndpoint* from)
    -> ISimulatedController*
{
    const auto fromParticipant = from->GetServiceDescriptor().GetParticipantName();
    const auto fromServiceId = from->GetServiceDescriptor().GetServiceId();
    auto it_simulatedControllersByParticipant = _simulatedControllers.find(fromParticipant);
    if (it_simulatedControllersByParticipant != _simulatedControllers.end())
    {
        auto it_simulatedControllersByServiceId = it_simulatedControllersByParticipant->second.find(fromServiceId);
        if (it_simulatedControllersByServiceId != it_simulatedControllersByParticipant->second.end())
        {
            return it_simulatedControllersByServiceId->second;
        }
    }
    _participant->GetLogger()->Error(
        "NetworkSimulation: No simulated controller was found to route message from participant '" + fromParticipant
        + "', serviceId " + std::to_string(fromServiceId));
    return {};
}

// --------------------------------
// Can
// --------------------------------

auto SimulatedNetworkRouter::GetSimulatedCanControllerFromServiceEndpoint(const SilKit::Core::IServiceEndpoint* from)
    -> Can::ISimulatedCanController*
{
    return static_cast<Can::ISimulatedCanController*>(GetSimulatedControllerFromServiceEndpoint(from));
}

void SimulatedNetworkRouter::ReceiveMsg(const SilKit::Core::IServiceEndpoint* from,
                                        const SilKit::Services::Can::WireCanFrameEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Can::CanFrameRequest netsimMsg;
    netsimMsg.frame = ToCanFrame(msg.frame);
    netsimMsg.userContext = msg.userContext;

    auto controller = GetSimulatedCanControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnFrameRequest(netsimMsg);
    }
}

void SimulatedNetworkRouter::ReceiveMsg(const SilKit::Core::IServiceEndpoint* from,
                                        const SilKit::Services::Can::CanConfigureBaudrate& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Can::CanConfigureBaudrate netsimMsg;
    netsimMsg.baudRate = msg.baudRate;
    netsimMsg.fdBaudRate = msg.fdBaudRate;
    netsimMsg.xlBaudRate = msg.xlBaudRate;

    auto controller = GetSimulatedCanControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnSetBaudrate(netsimMsg);
    }
}

void SimulatedNetworkRouter::ReceiveMsg(const SilKit::Core::IServiceEndpoint* from,
                                        const SilKit::Services::Can::CanSetControllerMode& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Can::CanControllerMode netsimMsg{};
    if (msg.flags.resetErrorHandling)
    {
        netsimMsg.canControllerModeFlags |= SilKit_Experimental_NetSim_CanControllerModeFlags_ResetErrorHandling;
    }
    if (msg.flags.cancelTransmitRequests)
    {
        netsimMsg.canControllerModeFlags |= SilKit_Experimental_NetSim_CanControllerModeFlags_CancelTransmitRequests;
    }
    netsimMsg.state = msg.mode;

    auto controller = GetSimulatedCanControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnSetControllerMode(netsimMsg);
    }
}

// --------------------------------
// FlexRay
// --------------------------------

auto SimulatedNetworkRouter::GetSimulatedFlexRayControllerFromServiceEndpoint(
    const SilKit::Core::IServiceEndpoint* from) -> Flexray::ISimulatedFlexRayController*
{
    return static_cast<Flexray::ISimulatedFlexRayController*>(GetSimulatedControllerFromServiceEndpoint(from));
}

void SimulatedNetworkRouter::ReceiveMsg(const Core::IServiceEndpoint* from,
                                        const SilKit::Services::Flexray::FlexrayHostCommand& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Flexray::FlexrayHostCommand netsimMsg;
    netsimMsg.command = static_cast<Flexray::FlexrayChiCommand>(msg.command);

    auto controller = GetSimulatedFlexRayControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnHostCommand(netsimMsg);
    }
}

void SimulatedNetworkRouter::ReceiveMsg(const Core::IServiceEndpoint* from,
                                        const SilKit::Services::Flexray::FlexrayControllerConfig& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Flexray::FlexrayControllerConfig netsimMsg;
    netsimMsg.bufferConfigs = msg.bufferConfigs;
    netsimMsg.clusterParams = msg.clusterParams;
    netsimMsg.nodeParams = msg.nodeParams;

    auto controller = GetSimulatedFlexRayControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnControllerConfig(netsimMsg);
    }
}

void SimulatedNetworkRouter::ReceiveMsg(const Core::IServiceEndpoint* from,
                                        const SilKit::Services::Flexray::FlexrayTxBufferConfigUpdate& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Flexray::FlexrayTxBufferConfigUpdate netsimMsg;
    netsimMsg.txBufferConfig = msg.txBufferConfig;
    netsimMsg.txBufferIndex = msg.txBufferIndex;

    auto controller = GetSimulatedFlexRayControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnTxBufferConfigUpdate(netsimMsg);
    }
}

void SimulatedNetworkRouter::ReceiveMsg(const Core::IServiceEndpoint* from,
                                        const SilKit::Services::Flexray::WireFlexrayTxBufferUpdate& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Flexray::FlexrayTxBufferUpdate netsimMsg;
    netsimMsg.payload = msg.payload.AsSpan();
    netsimMsg.payloadDataValid = msg.payloadDataValid;
    netsimMsg.txBufferIndex = msg.txBufferIndex;

    auto controller = GetSimulatedFlexRayControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnTxBufferUpdate(netsimMsg);
    }
}

// --------------------------------
// Ethernet
// --------------------------------

auto SimulatedNetworkRouter::GetSimulatedEthernetControllerFromServiceEndpoint(
    const SilKit::Core::IServiceEndpoint* from) -> Ethernet::ISimulatedEthernetController*
{
    return static_cast<Ethernet::ISimulatedEthernetController*>(GetSimulatedControllerFromServiceEndpoint(from));
}

void SimulatedNetworkRouter::ReceiveMsg(const Core::IServiceEndpoint* from,
                                        const SilKit::Services::Ethernet::WireEthernetFrameEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Ethernet::EthernetFrameRequest netsimMsg;
    netsimMsg.ethernetFrame = ToEthernetFrame(msg.frame);
    netsimMsg.userContext = msg.userContext;

    auto controller = GetSimulatedEthernetControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnFrameRequest(netsimMsg);
    }
}

void SimulatedNetworkRouter::ReceiveMsg(const Core::IServiceEndpoint* from,
                                        const SilKit::Services::Ethernet::EthernetSetMode& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Ethernet::EthernetControllerMode netsimMsg;
    switch (msg.mode)
    {
    case SilKit::Services::Ethernet::EthernetMode::Active:
        netsimMsg.mode = Ethernet::EthernetMode::Active;
        break;
    case SilKit::Services::Ethernet::EthernetMode::Inactive:
        netsimMsg.mode = Ethernet::EthernetMode::Inactive;
        break;
    default:
        break;
    }

    auto controller = GetSimulatedEthernetControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnSetControllerMode(netsimMsg);
    }
}

// --------------------------------
// Lin
// --------------------------------

auto SimulatedNetworkRouter::GetSimulatedLinControllerFromServiceEndpoint(const SilKit::Core::IServiceEndpoint* from)
    -> Lin::ISimulatedLinController*
{
    return static_cast<Lin::ISimulatedLinController*>(GetSimulatedControllerFromServiceEndpoint(from));
}

void SimulatedNetworkRouter::ReceiveMsg(const Core::IServiceEndpoint* from,
                                        const SilKit::Services::Lin::LinSendFrameRequest& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Lin::LinFrameRequest netsimMsg;
    netsimMsg.frame = msg.frame;
    netsimMsg.responseType = msg.responseType;

    auto controller = GetSimulatedLinControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnFrameRequest(netsimMsg);
    }
}

void SimulatedNetworkRouter::ReceiveMsg(const Core::IServiceEndpoint* from,
                                        const SilKit::Services::Lin::LinSendFrameHeaderRequest& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Lin::LinFrameHeaderRequest netsimMsg;
    netsimMsg.id = msg.id;

    auto controller = GetSimulatedLinControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnFrameHeaderRequest(netsimMsg);
    }
}

void SimulatedNetworkRouter::ReceiveMsg(const Core::IServiceEndpoint* from, const SilKit::Services::Lin::LinWakeupPulse& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Lin::LinWakeupPulse netsimMsg;
    netsimMsg.timestamp = msg.timestamp;

    auto controller = GetSimulatedLinControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnWakeupPulse(netsimMsg);
    }
}

void SimulatedNetworkRouter::ReceiveMsg(const Core::IServiceEndpoint* from,
                                        const SilKit::Services::Lin::WireLinControllerConfig& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Lin::LinControllerConfig netsimMsg;
    netsimMsg.baudRate = msg.baudRate;
    netsimMsg.controllerMode = msg.controllerMode;
    netsimMsg.frameResponses = msg.frameResponses;
    switch (msg.simulationMode)
    {
    case SilKit::Services::Lin::WireLinControllerConfig::SimulationMode::Default:
        netsimMsg.simulationMode = Lin::LinControllerConfig::SimulationMode::Default;
        break;
    case SilKit::Services::Lin::WireLinControllerConfig::SimulationMode::Dynamic:
        netsimMsg.simulationMode = Lin::LinControllerConfig::SimulationMode::Dynamic;
        break;
    default:
        break;
    }

    auto controller = GetSimulatedLinControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnControllerConfig(netsimMsg);
    }
}

void SimulatedNetworkRouter::ReceiveMsg(const Core::IServiceEndpoint* from,
                                        const SilKit::Services::Lin::LinFrameResponseUpdate& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Lin::LinFrameResponseUpdate netsimMsg;
    netsimMsg.frameResponses = msg.frameResponses;

    auto controller = GetSimulatedLinControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnFrameResponseUpdate(netsimMsg);
    }
}

void SimulatedNetworkRouter::ReceiveMsg(const Core::IServiceEndpoint* from,
                                        const SilKit::Services::Lin::LinControllerStatusUpdate& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    Lin::LinControllerStatusUpdate netsimMsg;
    netsimMsg.status = msg.status;
    netsimMsg.timestamp = msg.timestamp;

    auto controller = GetSimulatedLinControllerFromServiceEndpoint(from);
    if (controller)
    {
        controller->OnControllerStatusUpdate(netsimMsg);
    }
}

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit

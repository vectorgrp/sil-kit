// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <unordered_map>
#include <set>

#include "IParticipantInternal.hpp"
#include "ISimulator.hpp"
#include "IServiceEndpoint.hpp"

#include "silkit/experimental/netsim/all.hpp"

#include "WireCanMessages.hpp"
#include "WireEthernetMessages.hpp"
#include "WireFlexrayMessages.hpp"
#include "WireLinMessages.hpp"

#include "ILogger.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

class SimulatedNetworkRouter
    : public Core::ISimulator
{
public:
    SimulatedNetworkRouter(Core::IParticipantInternal* participant, const std::string& networkName,
                           SimulatedNetworkType networkType);

    // ISimulator

    // IMsgForCanSimulator::IReceiver
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Can::WireCanFrameEvent& msg) override;
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Can::CanConfigureBaudrate& msg) override;
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Can::CanSetControllerMode& msg) override;

    // IMsgForFrBusSimulator::IReceiver
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Flexray::FlexrayHostCommand& msg) override;
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Flexray::FlexrayControllerConfig& msg) override;
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Flexray::FlexrayTxBufferConfigUpdate& msg) override;
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Flexray::WireFlexrayTxBufferUpdate& msg) override;

    // IMsgForEthSimulator::IReceiver
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Ethernet::WireEthernetFrameEvent& msg) override;
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Ethernet::EthernetSetMode& msg) override;

    // IMsgForLinSimulator::IReceiver
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Lin::LinSendFrameRequest& msg) override;
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Lin::LinSendFrameHeaderRequest& msg) override;
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Lin::LinWakeupPulse& msg) override;
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Lin::WireLinControllerConfig& msg) override;
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Lin::LinFrameResponseUpdate& msg) override;
    virtual void ReceiveMsg(const Core::IServiceEndpoint* from,
                            const SilKit::Services::Lin::LinControllerStatusUpdate& msg) override;

    template <typename SilKitMessageT>
    void SendMsg(SilKitMessageT&& msg, const SilKit::Util::Span<const ControllerDescriptor>& receivers)
    {
        for (const auto& receiver : receivers)
        {
            auto targetController = _targetControllers.find(receiver);
            if (targetController != _targetControllers.end())
            {
                _participant->SendMsg(targetController->second.get(), targetController->second->participantName, msg);
            }
            else
            {
                _participant->GetLogger()->Warn("EventProvider has no receiving controller on network '" + _networkName + "'");
            }
        }
    }

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

    void AddSimulatedController(const std::string& fromParticipantName,
                                const std::string& controllerName, Core::EndpointId serviceId,
                                ControllerDescriptor controllerDescriptor,
                                ISimulatedController* userSimulatedController);
    void RemoveSimulatedController(const std::string& fromParticipantName, Core::EndpointId serviceId,
                                   ControllerDescriptor controllerDescriptor);

private :
    void AnnounceNetwork(const std::string& networkName, SimulatedNetworkType networkType);
    bool AllowReception(const SilKit::Core::IServiceEndpoint* from);
    auto GetSimulatedControllerFromServiceEndpoint(const SilKit::Core::IServiceEndpoint* from) 
        -> ISimulatedController*;

    auto GetSimulatedCanControllerFromServiceEndpoint(const SilKit::Core::IServiceEndpoint* from)
        -> Can::ISimulatedCanController*;
    auto GetSimulatedFlexRayControllerFromServiceEndpoint(const SilKit::Core::IServiceEndpoint* from)
        -> Flexray::ISimulatedFlexRayController*;
    auto GetSimulatedEthernetControllerFromServiceEndpoint(const SilKit::Core::IServiceEndpoint* from)
        -> Ethernet::ISimulatedEthernetController*;
    auto GetSimulatedLinControllerFromServiceEndpoint(const SilKit::Core::IServiceEndpoint* from)
        -> Lin::ISimulatedLinController*;

    Core::IParticipantInternal* _participant = nullptr;
    std::string _networkName;
    SimulatedNetworkType _networkType;

    struct TargetController : Core::IServiceEndpoint
    {
        std::string participantName;
        Core::ServiceDescriptor _serviceDescriptor{};
        void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override
        {
            _serviceDescriptor = serviceDescriptor;
        }
        auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override
        {
            return _serviceDescriptor;
        }
    };
    std::unordered_map<ControllerDescriptor, std::unique_ptr<TargetController>> _targetControllers;

    Core::ServiceDescriptor _serviceDescriptor{};

    // ServiceId is unique per participant
    using SimulatedControllersByServiceId = std::unordered_map<Core::EndpointId /*serviceId*/, ISimulatedController* /*controller*/>;
    using SimulatedControllersByParticipantAndServiceId =
        std::unordered_map<std::string /*participantName*/, SimulatedControllersByServiceId /*inner*/>;
    SimulatedControllersByParticipantAndServiceId _simulatedControllers;
};

// ------------------------------------------------------------------------
//  Inline Implementation
// ------------------------------------------------------------------------

inline void SimulatedNetworkRouter::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

inline auto SimulatedNetworkRouter::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit

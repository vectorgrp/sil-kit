// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"
#include "SimBehaviorDetailed.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

SimBehaviorDetailed::SimBehaviorDetailed(Core::IParticipantInternal* participant, EthController* ethController,
                                       const Core::ServiceDescriptor& serviceDescriptor)
    : _participant{participant}
    , _parentServiceEndpoint{dynamic_cast<Core::IServiceEndpoint*>(ethController)}
    , _parentServiceDescriptor{&serviceDescriptor}
{
}

template <typename MsgT>
void SimBehaviorDetailed::SendMsgImpl(MsgT&& msg)
{
    _participant->SendMsg(_parentServiceEndpoint, std::forward<MsgT>(msg));
}

void SimBehaviorDetailed::SendMsg(WireEthernetFrameEvent&& msg)
{
    // We keep a copy until the transmission was acknowledged before tracing the message
    _transmittedMessages[msg.transmitId] = msg.frame;

    SendMsgImpl(msg);
}

void SimBehaviorDetailed::SendMsg(EthernetSetMode&& msg)
{
    SendMsgImpl(msg);
}

void SimBehaviorDetailed::OnReceiveAck(const EthernetFrameTransmitEvent& msg)
{
    // Detailed Sim: Check if frame originates from this controller to trace the correct direction
    auto transmittedMsg = _transmittedMessages.find(msg.transmitId);
    if (transmittedMsg != _transmittedMessages.end())
    {
        if (msg.status == EthernetTransmitStatus::Transmitted)
        {
            _tracer.Trace(SilKit::Services::TransmitDirection::TX, msg.timestamp, ToEthernetFrame(transmittedMsg->second));
        }

        _transmittedMessages.erase(msg.transmitId);
    }
}

auto SimBehaviorDetailed::AllowReception(const Core::IServiceEndpoint* from) const -> bool
{
    // If simulated, only allow reception from NetSim.
    // NetSim internally sets the ServiceId of this controller and sends messages with it,
    // this controller knows about NetSim through _simulatedLink.
    const auto& fromDescr = from->GetServiceDescriptor();
    return _simulatedLink.GetParticipantName() == fromDescr.GetParticipantName()
           && _parentServiceDescriptor->GetServiceId() == fromDescr.GetServiceId();
}

void SimBehaviorDetailed::SetSimulatedLink(const Core::ServiceDescriptor& simulatedLink)
{
    _simulatedLink = simulatedLink;
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit

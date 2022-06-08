// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"
#include "SimBehaviorDetailed.hpp"

namespace ib {
namespace sim {
namespace eth {

SimBehaviorDetailed::SimBehaviorDetailed(mw::IParticipantInternal* participant, EthController* ethController,
                                       const mw::ServiceDescriptor& serviceDescriptor)
    : _participant{participant}
    , _parentServiceEndpoint{dynamic_cast<mw::IIbServiceEndpoint*>(ethController)}
    , _parentServiceDescriptor{&serviceDescriptor}
{
}

template <typename MsgT>
void SimBehaviorDetailed::SendIbMessageImpl(MsgT&& msg)
{
    _participant->SendIbMessage(_parentServiceEndpoint, std::forward<MsgT>(msg));
}

void SimBehaviorDetailed::SendIbMessage(EthernetFrameEvent&& msg)
{
    // We keep a copy until the transmission was acknowledged before tracing the message
    _transmittedMessages[msg.transmitId] = msg.frame;

    SendIbMessageImpl(msg);
}

void SimBehaviorDetailed::SendIbMessage(EthernetSetMode&& msg)
{
    SendIbMessageImpl(msg);
}

void SimBehaviorDetailed::OnReceiveAck(const EthernetFrameTransmitEvent& msg)
{
    // Detailed Sim: Check if frame originates from this controller to trace the correct direction
    auto transmittedMsg = _transmittedMessages.find(msg.transmitId);
    if (transmittedMsg != _transmittedMessages.end())
    {
        if (msg.status == EthernetTransmitStatus::Transmitted)
        {
            _tracer.Trace(ib::sim::TransmitDirection::TX, msg.timestamp, transmittedMsg->second);
        }

        _transmittedMessages.erase(msg.transmitId);
    }
}

auto SimBehaviorDetailed::AllowReception(const mw::IIbServiceEndpoint* from) const -> bool 
{
    // If simulated, only allow reception from NetSim.
    // NetSim internally sets the ServiceId of this controller and sends messages with it,
    // this controller knows about NetSim through _simulatedLink.
    const auto& fromDescr = from->GetServiceDescriptor();
    return _simulatedLink.GetParticipantName() == fromDescr.GetParticipantName()
           && _parentServiceDescriptor->GetServiceId() == fromDescr.GetServiceId();
}

void SimBehaviorDetailed::SetSimulatedLink(const mw::ServiceDescriptor& simulatedLink)
{
    _simulatedLink = simulatedLink;
}

} // namespace eth
} // namespace sim
} // namespace ib

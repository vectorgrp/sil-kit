// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanController.hpp"
#include "SimBehaviorDetailed.hpp"

namespace SilKit {
namespace Services {
namespace Can {

SimBehaviorDetailed::SimBehaviorDetailed(Core::IParticipantInternal* participant, CanController* canController,
                                       const Core::ServiceDescriptor& serviceDescriptor)
    : _participant{participant}
    , _parentServiceEndpoint{dynamic_cast<Core::IServiceEndpoint*>(canController)}
    , _parentServiceDescriptor{&serviceDescriptor}
{
}

template <typename MsgT>
void SimBehaviorDetailed::SendMsgImpl(MsgT&& msg)
{
    _participant->SendMsg(_parentServiceEndpoint, std::forward<MsgT>(msg));
}
void SimBehaviorDetailed::SendMsg(CanConfigureBaudrate&& msg)
{
    SendMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(CanSetControllerMode&& msg)
{
    SendMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(WireCanFrameEvent&& msg)
{
    SendMsgImpl(msg);
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

} // namespace Can
} // namespace Services
} // namespace SilKit

// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanController.hpp"
#include "SimBehaviorDetailed.hpp"

namespace ib {
namespace sim {
namespace can {

SimBehaviorDetailed::SimBehaviorDetailed(mw::IParticipantInternal* participant, CanController* canController,
                                       const mw::ServiceDescriptor& serviceDescriptor)
    : _participant{participant}
    , _parentController{canController}
    , _parentServiceEndpoint{dynamic_cast<mw::IIbServiceEndpoint*>(canController)}
    , _parentServiceDescriptor{&serviceDescriptor}
{
    (void)_parentController;
}

template <typename MsgT>
void SimBehaviorDetailed::SendIbMessageImpl(MsgT&& msg)
{
    _participant->SendIbMessage(_parentServiceEndpoint, std::forward<MsgT>(msg));
}
void SimBehaviorDetailed::SendIbMessage(CanConfigureBaudrate&& msg)
{
    SendIbMessageImpl(msg);
}
void SimBehaviorDetailed::SendIbMessage(CanSetControllerMode&& msg)
{
    SendIbMessageImpl(msg);
}
void SimBehaviorDetailed::SendIbMessage(CanFrameEvent&& msg)
{
    SendIbMessageImpl(msg);
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

} // namespace can
} // namespace sim
} // namespace ib

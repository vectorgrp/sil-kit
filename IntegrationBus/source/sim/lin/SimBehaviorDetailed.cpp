// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"
#include "SimBehaviorDetailed.hpp"

namespace ib {
namespace sim {
namespace lin {

SimBehaviorDetailed::SimBehaviorDetailed(mw::IParticipantInternal* participant, LinController* linController,
                                       const mw::ServiceDescriptor& serviceDescriptor)
    : _participant{participant}
    , _parentController{linController}
    , _parentServiceEndpoint{dynamic_cast<mw::IIbServiceEndpoint*>(linController)}
    , _parentServiceDescriptor{&serviceDescriptor}
{
}

template <typename MsgT>
void SimBehaviorDetailed::SendIbMessageImpl(MsgT&& msg)
{
    _participant->SendIbMessage(_parentServiceEndpoint, std::forward<MsgT>(msg));
}

void SimBehaviorDetailed::SendIbMessage(LinSendFrameRequest&& msg)
{
    SendIbMessageImpl(msg);
}
void SimBehaviorDetailed::SendIbMessage(LinTransmission&& msg)
{
    SendIbMessageImpl(msg);
}
void SimBehaviorDetailed::SendIbMessage(LinControllerConfig&& msg)
{
    SendIbMessageImpl(msg);
}
void SimBehaviorDetailed::SendIbMessage(LinSendFrameHeaderRequest&& msg)
{
    SendIbMessageImpl(msg);
}
void SimBehaviorDetailed::SendIbMessage(LinFrameResponseUpdate&& msg)
{
    SendIbMessageImpl(msg);
}
void SimBehaviorDetailed::SendIbMessage(LinControllerStatusUpdate&& msg)
{
    SendIbMessageImpl(msg);
}

void SimBehaviorDetailed::GoToSleep()
{
    LinSendFrameRequest gotosleepFrame;
    gotosleepFrame.frame = GoToSleepFrame();
    gotosleepFrame.responseType = LinFrameResponseType::MasterResponse;

    SendIbMessageImpl(gotosleepFrame);

    // We signal SleepPending to the network simulator, so it will be able
    // to finish sleep frame transmissions before entering Sleep state.
    // cf. AUTOSAR SWS LIN Driver section 7.3.3 [SWS_Lin_00263].
    // Locally, the _controllerStatus is set to Sleep in the LinController
    // so we don't expose the internal SleepPending state to users.
    _parentController->SetControllerStatus(LinControllerStatus::SleepPending);
}

void SimBehaviorDetailed::Wakeup()
{
    // Send without direction, netsim will distribute with correct directions
    LinWakeupPulse pulse{};
    SendIbMessageImpl(pulse);
    _parentController->WakeupInternal();
}

auto SimBehaviorDetailed::CalcFrameStatus(const LinTransmission& linTransmission, bool /*isGoToSleepFrame*/)
    -> LinFrameStatus
{
    return linTransmission.status;
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

} // namespace lin
} // namespace sim
} // namespace ib

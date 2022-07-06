// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"
#include "SimBehaviorDetailed.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

SimBehaviorDetailed::SimBehaviorDetailed(Core::IParticipantInternal* participant, LinController* linController,
                                       const Core::ServiceDescriptor& serviceDescriptor)
    : _participant{participant}
    , _parentController{linController}
    , _parentServiceEndpoint{dynamic_cast<Core::IServiceEndpoint*>(linController)}
    , _parentServiceDescriptor{&serviceDescriptor}
{
}

template <typename MsgT>
void SimBehaviorDetailed::SendMsgImpl(MsgT&& msg)
{
    _participant->SendMsg(_parentServiceEndpoint, std::forward<MsgT>(msg));
}

void SimBehaviorDetailed::SendMsg(LinSendFrameRequest&& msg)
{
    SendMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(LinTransmission&& msg)
{
    SendMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(LinControllerConfig&& msg)
{
    SendMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(LinSendFrameHeaderRequest&& msg)
{
    SendMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(LinFrameResponseUpdate&& msg)
{
    SendMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(LinControllerStatusUpdate&& msg)
{
    SendMsgImpl(msg);
}

void SimBehaviorDetailed::GoToSleep()
{
    LinSendFrameRequest gotosleepFrame;
    gotosleepFrame.frame = GoToSleepFrame();
    gotosleepFrame.responseType = LinFrameResponseType::MasterResponse;

    SendMsgImpl(gotosleepFrame);

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
    SendMsgImpl(pulse);
    _parentController->WakeupInternal();
}

auto SimBehaviorDetailed::CalcFrameStatus(const LinTransmission& linTransmission, bool /*isGoToSleepFrame*/)
    -> LinFrameStatus
{
    return linTransmission.status;
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

} // namespace Lin
} // namespace Services
} // namespace SilKit

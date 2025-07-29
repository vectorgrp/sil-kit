// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
void SimBehaviorDetailed::SendBroadcastMsgImpl(MsgT&& msg)
{
    _participant->SendMsg(_parentServiceEndpoint, std::forward<MsgT>(msg));
}

template <typename MsgT>
void SimBehaviorDetailed::SendTargettedMsgImpl(MsgT&& msg)
{
    _participant->SendMsg(_parentServiceEndpoint, _simulatedLink.GetParticipantName(), std::forward<MsgT>(msg));
}

void SimBehaviorDetailed::SendMsg(LinSendFrameRequest&& msg)
{
    SendTargettedMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(LinTransmission&& msg)
{
    SendTargettedMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(LinSendFrameHeaderRequest&& msg)
{
    SendTargettedMsgImpl(msg);
}

void SimBehaviorDetailed::SendMsg(WireLinControllerConfig&& msg)
{
    SendBroadcastMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(LinFrameResponseUpdate&& msg)
{
    SendBroadcastMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(LinControllerStatusUpdate&& msg)
{
    SendBroadcastMsgImpl(msg);
}

void SimBehaviorDetailed::ProcessFrameHeaderRequest(const LinSendFrameHeaderRequest& /*header*/)
{
    // NOP
}

void SimBehaviorDetailed::UpdateTxBuffer(const LinFrame& frame)
{
    LinFrameResponseUpdate responseUpdate{};
    LinFrameResponse response{};
    response.frame = frame;
    response.responseMode = LinFrameResponseMode::TxUnconditional;
    responseUpdate.frameResponses.push_back(response);
    SendBroadcastMsgImpl(responseUpdate);
}

void SimBehaviorDetailed::GoToSleep()
{
    LinSendFrameRequest gotosleepFrame;
    gotosleepFrame.frame = GoToSleepFrame();
    gotosleepFrame.responseType = LinFrameResponseType::MasterResponse;

    SendTargettedMsgImpl(gotosleepFrame);

    // We signal SleepPending to the network simulator, so it will be able
    // to finish sleep frame transmissions before entering Sleep state.
    // cf. AUTOSAR SWS LIN Driver section 7.3.3 [SWS_Lin_00263].
    // Locally, the _controllerStatus is set to Sleep in the LinController
    // so we don't expose the internal SleepPending state to users.
    _parentController->SetControllerStatusInternal(LinControllerStatus::SleepPending);
}

void SimBehaviorDetailed::Wakeup()
{
    // Send without direction, netsim will distribute with correct directions
    LinWakeupPulse pulse{};
    SendTargettedMsgImpl(pulse);
    _parentController->WakeupInternal();
}

auto SimBehaviorDetailed::CalcFrameStatus(const LinTransmission& linTransmission,
                                          bool isGoToSleepFrame) -> LinFrameStatus
{
    // dynamic controllers report every transmission as it was received
    if (_parentController->GetThisLinNode().simulationMode == WireLinControllerConfig::SimulationMode::Dynamic)
    {
        return linTransmission.status;
    }

    // If GoToSleepFrame comes with RX_OK, use only if configured for RX
    if (isGoToSleepFrame && linTransmission.status == LinFrameStatus::LIN_RX_OK)
    {
        auto& thisLinNode = _parentController->GetThisLinNode();
        const auto response = thisLinNode.responses[linTransmission.frame.id];
        if (response.responseMode != LinFrameResponseMode::Rx)
        {
            return LinFrameStatus::LIN_RX_NO_RESPONSE;
        }
    }

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

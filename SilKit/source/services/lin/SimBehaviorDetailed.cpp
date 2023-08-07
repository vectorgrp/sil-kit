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
void SimBehaviorDetailed::SendMsg(WireLinControllerConfig&& msg)
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
    SendMsgImpl(responseUpdate);
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
    _parentController->SetControllerStatusInternal(LinControllerStatus::SleepPending);
}

void SimBehaviorDetailed::Wakeup()
{
    // Send without direction, netsim will distribute with correct directions
    LinWakeupPulse pulse{};
    SendMsgImpl(pulse);
    _parentController->WakeupInternal();
}

auto SimBehaviorDetailed::CalcFrameStatus(const LinTransmission& linTransmission, bool isGoToSleepFrame)
    -> LinFrameStatus
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

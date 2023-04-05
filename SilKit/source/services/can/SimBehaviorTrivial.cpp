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

#include "CanController.hpp"
#include "SimBehaviorTrivial.hpp" 
#include "Assert.hpp"

#include "silkit/services/logging/ILogger.hpp"




namespace SilKit {
namespace Services {
namespace Can {

SimBehaviorTrivial::SimBehaviorTrivial(Core::IParticipantInternal* participant, CanController* canController,
                    Services::Orchestration::ITimeProvider* timeProvider)
    : _participant{participant}
    , _parentController{canController}
    , _parentServiceEndpoint{dynamic_cast<Core::IServiceEndpoint*>(canController)}
    , _timeProvider{timeProvider}
{
    (void)_parentController;
}

template <typename MsgT>
void SimBehaviorTrivial::ReceiveMsg(const MsgT& msg)
{
    auto receivingController = dynamic_cast<Core::IMessageReceiver<MsgT>*>(_parentController);
    SILKIT_ASSERT(receivingController);
    receivingController->ReceiveMsg(_parentServiceEndpoint, msg);
}

auto SimBehaviorTrivial::AllowReception(const Core::IServiceEndpoint* /*from*/) const -> bool 
{ 
    return true; 
}

void SimBehaviorTrivial::SendMsg(CanConfigureBaudrate&& /*baudRate*/)
{ 
}

void SimBehaviorTrivial::SendMsg(CanSetControllerMode&& mode)
{
    CanControllerStatus newStatus{};
    newStatus.timestamp = _timeProvider->Now();
    newStatus.controllerState = mode.mode;

    ReceiveMsg(newStatus);
}

void SimBehaviorTrivial::SendMsg(WireCanFrameEvent&& canFrameEvent)
{
    if (_parentController->GetState() == CanControllerState::Started)
    {
        auto now = _timeProvider->Now();
        WireCanFrameEvent canFrameEventCpy = canFrameEvent;
        canFrameEventCpy.timestamp = now;

        // Send to others as RX
        canFrameEventCpy.direction = TransmitDirection::RX;
        _participant->SendMsg(_parentServiceEndpoint, canFrameEventCpy);

        // Self delivery as TX (handles TX tracing)
        canFrameEventCpy.direction = TransmitDirection::TX;
        ReceiveMsg(canFrameEventCpy);

        // Self acknowledge
        CanFrameTransmitEvent ack{};
        ack.canId = canFrameEvent.frame.canId;
        ack.status = CanTransmitStatus::Transmitted;
        ack.userContext = canFrameEvent.userContext;
        ack.timestamp = now;

        ReceiveMsg(ack);
    }
    else
    {
        _participant->GetLogger()->Warn("ICanController::SendFrame is called although can controller is not in state CanController::Started.");
    }
}

} // namespace Can
} // namespace Services
} // namespace SilKit

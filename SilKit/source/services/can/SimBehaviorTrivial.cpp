// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

void SimBehaviorTrivial::SendMsg(CanConfigureBaudrate&& /*baudRate*/) {}

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
        _participant->GetLogger()->Warn(
            "ICanController::SendFrame is called although can controller is not in state CanController::Started.");
    }
}

} // namespace Can
} // namespace Services
} // namespace SilKit

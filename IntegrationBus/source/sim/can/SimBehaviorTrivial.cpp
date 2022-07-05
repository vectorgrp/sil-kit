// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanController.hpp"
#include "SimBehaviorTrivial.hpp" 
#include "ib/mw/logging/ILogger.hpp"


namespace ib {
namespace sim {
namespace can {

SimBehaviorTrivial::SimBehaviorTrivial(mw::IParticipantInternal* participant, CanController* canController,
                    mw::sync::ITimeProvider* timeProvider)
    : _participant{participant}
    , _parentController{canController}
    , _parentServiceEndpoint{dynamic_cast<mw::IIbServiceEndpoint*>(canController)}
    , _timeProvider{timeProvider}
{
    (void)_parentController;
}

template <typename MsgT>
void SimBehaviorTrivial::ReceiveIbMessage(const MsgT& msg)
{
    auto receivingController = dynamic_cast<mw::IIbMessageReceiver<MsgT>*>(_parentController);
    assert(receivingController);
    receivingController->ReceiveIbMessage(_parentServiceEndpoint, msg);
}

auto SimBehaviorTrivial::AllowReception(const mw::IIbServiceEndpoint* /*from*/) const -> bool 
{ 
    return true; 
}

void SimBehaviorTrivial::SendIbMessage(CanConfigureBaudrate&& /*baudRate*/)
{ 
}

void SimBehaviorTrivial::SendIbMessage(CanSetControllerMode&& mode)
{
    CanControllerStatus newStatus;
    newStatus.timestamp = _timeProvider->Now();
    newStatus.controllerState = mode.mode;

    ReceiveIbMessage(newStatus);
}

void SimBehaviorTrivial::SendIbMessage(CanFrameEvent&& canFrameEvent)
{
    if (_parentController->GetState() == CanControllerState::Started)
    {
        auto now = _timeProvider->Now();
        CanFrameEvent canFrameEventCpy = canFrameEvent;
        canFrameEventCpy.direction = TransmitDirection::TX;
        canFrameEventCpy.timestamp = now;

        _tracer.Trace(ib::sim::TransmitDirection::TX, now, canFrameEvent);

        // Self delivery as TX
        ReceiveIbMessage(canFrameEventCpy);

        // Send to others as RX
        canFrameEventCpy.direction = TransmitDirection::RX;
        _participant->SendIbMessage(_parentServiceEndpoint, canFrameEventCpy);

        // Self acknowledge
        CanFrameTransmitEvent ack{};
        ack.canId = canFrameEvent.frame.canId;
        ack.status = CanTransmitStatus::Transmitted;
        ack.transmitId = canFrameEvent.transmitId;
        ack.userContext = canFrameEvent.userContext;
        ack.timestamp = now;

        ReceiveIbMessage(ack);
    }
    else
    {
        _participant->GetLogger()->Warn("ICanController::SendFrame is called although can controller is not in state CanController::Started.");
    }
}

} // namespace can
} // namespace sim
} // namespace ib

// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"
#include "SimBehavior.hpp"

namespace ib {
namespace sim {
namespace lin {

class LinController;

SimBehavior::SimBehavior(mw::IParticipantInternal* participant, LinController* linController,
                    mw::sync::ITimeProvider* timeProvider)
    : _trivial{participant, linController, timeProvider}
    , _detailed{participant, linController, linController->GetServiceDescriptor()}
{
    _currentBehavior = &_trivial;
}

auto SimBehavior::AllowReception(const mw::IIbServiceEndpoint* from) const -> bool
{
    return _currentBehavior->AllowReception(from);
}

template <typename MsgT>
void SimBehavior::SendIbMessageImpl(MsgT&& msg)
{
    _currentBehavior->SendIbMessage(std::forward<MsgT>(msg));
}

void SimBehavior::SendIbMessage(LinSendFrameRequest&& msg)
{
    SendIbMessageImpl(std::move(msg));
}
void SimBehavior::SendIbMessage(LinTransmission&& msg)
{
    SendIbMessageImpl(std::move(msg));
}
void SimBehavior::SendIbMessage(LinControllerConfig&& msg)
{
    SendIbMessageImpl(std::move(msg));
}
void SimBehavior::SendIbMessage(LinSendFrameHeaderRequest&& msg)
{
    SendIbMessageImpl(std::move(msg));
}
void SimBehavior::SendIbMessage(LinFrameResponseUpdate&& msg)
{
    SendIbMessageImpl(std::move(msg));
}
void SimBehavior::SendIbMessage(LinControllerStatusUpdate&& msg)
{
    SendIbMessageImpl(std::move(msg));
}

void SimBehavior::GoToSleep()
{
    _currentBehavior->GoToSleep();
}

void SimBehavior::Wakeup()
{
    _currentBehavior->Wakeup();
}

auto SimBehavior::CalcFrameStatus(const LinTransmission& linTransmission, bool isGoToSleepFrame) -> LinFrameStatus
{
    return _currentBehavior->CalcFrameStatus(linTransmission, isGoToSleepFrame);
}

void SimBehavior::SetDetailedBehavior(const mw::ServiceDescriptor& simulatedLink)
{
    _detailed.SetSimulatedLink(simulatedLink);
    _currentBehavior = &_detailed;
}
void SimBehavior::SetTrivialBehavior()
{
    _currentBehavior = &_trivial;
}

auto SimBehavior::IsTrivial() const -> bool
{
    return _currentBehavior == &_trivial;
}

auto SimBehavior::IsDetailed() const -> bool
{
    return _currentBehavior == &_detailed;
}


} // namespace lin
} // namespace sim
} // namespace ib

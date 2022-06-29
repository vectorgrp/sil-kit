// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanController.hpp"
#include "SimBehavior.hpp"

namespace ib {
namespace sim {
namespace can {

class CanController;

SimBehavior::SimBehavior(mw::IParticipantInternal* participant, CanController* canController,
                    mw::sync::ITimeProvider* timeProvider)
    : _trivial{participant, canController, timeProvider}
    , _detailed{participant, canController, canController->GetServiceDescriptor()}
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

void SimBehavior::SendIbMessage(CanConfigureBaudrate&& msg)
{
    SendIbMessageImpl(std::move(msg));
}

void SimBehavior::SendIbMessage(CanSetControllerMode&& msg)
{
    SendIbMessageImpl(std::move(msg));
}

void SimBehavior::SendIbMessage(CanFrameEvent&& msg)
{
    SendIbMessageImpl(std::move(msg));
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


} // namespace can
} // namespace sim
} // namespace ib

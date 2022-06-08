// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"
#include "SimBehavior.hpp"

namespace ib {
namespace sim {
namespace eth {

class EthController;

SimBehavior::SimBehavior(mw::IParticipantInternal* participant, EthController* ethController,
                    mw::sync::ITimeProvider* timeProvider)
    : _trivial{participant, ethController, timeProvider}
    , _detailed{participant, ethController, ethController->GetServiceDescriptor()}
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

void SimBehavior::SendIbMessage(EthernetFrameEvent&& msg) 
{ 
    SendIbMessageImpl(std::move(msg)); 
}

void SimBehavior::SendIbMessage(EthernetSetMode&& msg)
{
    SendIbMessageImpl(std::move(msg));
}

void SimBehavior::OnReceiveAck(const EthernetFrameTransmitEvent& msg)
{
    _currentBehavior->OnReceiveAck(msg);
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


} // namespace eth
} // namespace sim
} // namespace ib

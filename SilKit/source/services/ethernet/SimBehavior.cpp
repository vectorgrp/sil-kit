// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "EthController.hpp"
#include "SimBehavior.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

class EthController;

SimBehavior::SimBehavior(Core::IParticipantInternal* participant, EthController* ethController,
                         Services::Orchestration::ITimeProvider* timeProvider)
    : _trivial{participant, ethController, timeProvider}
    , _detailed{participant, ethController, ethController->GetServiceDescriptor()}
{
    _currentBehavior = &_trivial;
}

auto SimBehavior::AllowReception(const Core::IServiceEndpoint* from) const -> bool
{
    return _currentBehavior->AllowReception(from);
}

template <typename MsgT>
void SimBehavior::SendMsgImpl(MsgT&& msg)
{
    _currentBehavior->SendMsg(std::forward<MsgT>(msg));
}

void SimBehavior::SendMsg(WireEthernetFrameEvent&& msg)
{
    SendMsgImpl(std::move(msg));
}

void SimBehavior::SendMsg(EthernetSetMode&& msg)
{
    SendMsgImpl(std::move(msg));
}

void SimBehavior::OnReceiveAck(const EthernetFrameTransmitEvent& msg)
{
    _currentBehavior->OnReceiveAck(msg);
}

void SimBehavior::SetDetailedBehavior(const Core::ServiceDescriptor& simulatedLink)
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


} // namespace Ethernet
} // namespace Services
} // namespace SilKit

// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "CanController.hpp"
#include "SimBehavior.hpp"

namespace SilKit {
namespace Services {
namespace Can {

class CanController;

SimBehavior::SimBehavior(Core::IParticipantInternal* participant, CanController* canController,
                         Services::Orchestration::ITimeProvider* timeProvider)
    : _trivial{participant, canController, timeProvider}
    , _detailed{participant, canController, canController->GetServiceDescriptor()}
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

void SimBehavior::SendMsg(CanConfigureBaudrate&& msg)
{
    SendMsgImpl(std::move(msg));
}

void SimBehavior::SendMsg(CanSetControllerMode&& msg)
{
    SendMsgImpl(std::move(msg));
}

void SimBehavior::SendMsg(WireCanFrameEvent&& msg)
{
    SendMsgImpl(std::move(msg));
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


} // namespace Can
} // namespace Services
} // namespace SilKit

// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "I2cController.hpp"
#include "SimBehavior.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

SimBehavior::SimBehavior(Core::IParticipantInternal* participant, I2cController* controller,
                         Services::Orchestration::ITimeProvider* timeProvider)
    : _trivial{participant, controller, timeProvider}
    , _detailed{participant, controller, controller->GetServiceDescriptor()}
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

void SimBehavior::SendMsg(WireI2cFrameEvent&& msg)
{
    SendMsgImpl(std::move(msg));
}

void SimBehavior::SendMsg(WireI2cControllerConfig&& msg)
{
    SendMsgImpl(std::move(msg));
}

void SimBehavior::OnControllerConfig(const Core::IServiceEndpoint* from, const WireI2cControllerConfig& config)
{
    _currentBehavior->OnControllerConfig(from, config);
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

} // namespace I2c
} // namespace Services
} // namespace SilKit

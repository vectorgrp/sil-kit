// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "LinController.hpp"
#include "SimBehavior.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

class LinController;

SimBehavior::SimBehavior(Core::IParticipantInternal* participant, LinController* linController,
                         Services::Orchestration::ITimeProvider* timeProvider)
    : _trivial{participant, linController, timeProvider}
    , _detailed{participant, linController, linController->GetServiceDescriptor()}
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

void SimBehavior::SendMsg(LinSendFrameRequest&& msg)
{
    SendMsgImpl(std::move(msg));
}
void SimBehavior::SendMsg(LinTransmission&& msg)
{
    SendMsgImpl(std::move(msg));
}
void SimBehavior::SendMsg(WireLinControllerConfig&& msg)
{
    SendMsgImpl(std::move(msg));
}
void SimBehavior::SendMsg(LinSendFrameHeaderRequest&& msg)
{
    SendMsgImpl(std::move(msg));
}
void SimBehavior::SendMsg(LinFrameResponseUpdate&& msg)
{
    SendMsgImpl(std::move(msg));
}
void SimBehavior::SendMsg(LinControllerStatusUpdate&& msg)
{
    SendMsgImpl(std::move(msg));
}

void SimBehavior::ProcessFrameHeaderRequest(const LinSendFrameHeaderRequest& header)
{
    _currentBehavior->ProcessFrameHeaderRequest(header);
}

void SimBehavior::UpdateTxBuffer(const LinFrame& frame)
{
    _currentBehavior->UpdateTxBuffer(frame);
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


} // namespace Lin
} // namespace Services
} // namespace SilKit

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

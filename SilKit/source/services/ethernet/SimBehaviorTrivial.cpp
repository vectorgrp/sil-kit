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

#include "EthController.hpp"
#include "SimBehaviorTrivial.hpp"

namespace {

using SilKit::Services::Ethernet::EthernetState;
using SilKit::Services::Ethernet::EthernetTransmitStatus;

auto ControllerStateToTransmitStatus(const EthernetState ethernetState) -> EthernetTransmitStatus
{
    switch (ethernetState) {
    case EthernetState::LinkUp:
        return EthernetTransmitStatus::Transmitted;
    case EthernetState::LinkDown:
        return EthernetTransmitStatus::LinkDown;
    case EthernetState::Inactive:
        return EthernetTransmitStatus::ControllerInactive;
    }

    throw SilKit::StateError{"unknown EthernetState value"};
}

} // namespace

namespace SilKit {
namespace Services {
namespace Ethernet {

SimBehaviorTrivial::SimBehaviorTrivial(Core::IParticipantInternal* participant, EthController* ethController,
                    Services::Orchestration::ITimeProvider* timeProvider)
    : _participant{participant}
    , _parentController{ethController}
    , _parentServiceEndpoint{dynamic_cast<Core::IServiceEndpoint*>(ethController)}
    , _timeProvider{timeProvider}
{
    (void)_parentController;
}

template <typename MsgT>
void SimBehaviorTrivial::ReceiveMsg(const MsgT& msg)
{
    auto receivingController = static_cast<Core::IMessageReceiver<MsgT> *>(_parentController);
    receivingController->ReceiveMsg(_parentServiceEndpoint, msg);
}

auto SimBehaviorTrivial::AllowReception(const Core::IServiceEndpoint* /*from*/) const -> bool 
{ 
    return true; 
}

void SimBehaviorTrivial::SendMsg(WireEthernetFrameEvent&& ethFrameEvent)
{
    EthernetState controllerState = _parentController->GetState();

    // Trivial Sim: Set the timestamp, trace, send out the event and directly generate the ack
    ethFrameEvent.timestamp = _timeProvider->Now();

    if (controllerState == EthernetState::LinkUp)
    {
        // Send to others as RX
        ethFrameEvent.direction = TransmitDirection::RX;
        _participant->SendMsg(_parentServiceEndpoint, ethFrameEvent);

        // Self delivery as TX (handles TX tracing)
        ethFrameEvent.direction = TransmitDirection::TX;
        ReceiveMsg(ethFrameEvent);
    }

    EthernetFrameTransmitEvent ack;
    ack.timestamp = ethFrameEvent.timestamp;
    ack.status = ControllerStateToTransmitStatus(controllerState);
    ack.userContext = ethFrameEvent.userContext;
    ReceiveMsg(ack);
}

void SimBehaviorTrivial::SendMsg(EthernetSetMode&& ethSetMode)
{
    // Trivial: Reply EthernetSetMode locally with an EthernetStatus
    EthernetStatus statusReply{};
    statusReply.timestamp = _timeProvider->Now();

    if (ethSetMode.mode == EthernetMode::Active)
    {
        statusReply.state = EthernetState::LinkUp;
    }
    else if (ethSetMode.mode == EthernetMode::Inactive)
    {
        statusReply.state = EthernetState::Inactive;
    }
    ReceiveMsg(statusReply);
}

void SimBehaviorTrivial::OnReceiveAck(const EthernetFrameTransmitEvent& /*msg*/)
{
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit

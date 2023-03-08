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

#include "CanController.hpp"
#include "SimBehaviorDetailed.hpp"

namespace SilKit {
namespace Services {
namespace Can {

SimBehaviorDetailed::SimBehaviorDetailed(Core::IParticipantInternal* participant, CanController* canController,
                                       const Core::ServiceDescriptor& serviceDescriptor)
    : _participant{participant}
    , _parentServiceEndpoint{dynamic_cast<Core::IServiceEndpoint*>(canController)}
    , _parentServiceDescriptor{&serviceDescriptor}
    , _tracer{canController->GetTracer()}
{
}

template <typename MsgT>
void SimBehaviorDetailed::SendMsgImpl(MsgT&& msg)
{
    _participant->SendMsg(_parentServiceEndpoint, std::forward<MsgT>(msg));
}
void SimBehaviorDetailed::SendMsg(CanConfigureBaudrate&& msg)
{
    SendMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(CanSetControllerMode&& msg)
{
    SendMsgImpl(msg);
}
void SimBehaviorDetailed::SendMsg(WireCanFrameEvent&& msg)
{
    _tracer->Trace(msg.direction, msg.timestamp, ToCanFrameEvent(msg));
    SendMsgImpl(msg);
}

auto SimBehaviorDetailed::AllowReception(const Core::IServiceEndpoint* from) const -> bool 
{
    // If simulated, only allow reception from NetSim.
    // NetSim internally sets the ServiceId of this controller and sends messages with it,
    // this controller knows about NetSim through _simulatedLink.
    const auto& fromDescr = from->GetServiceDescriptor();
    return _simulatedLink.GetParticipantName() == fromDescr.GetParticipantName()
           && _parentServiceDescriptor->GetServiceId() == fromDescr.GetServiceId();
}

void SimBehaviorDetailed::SetSimulatedLink(const Core::ServiceDescriptor& simulatedLink)
{
    _simulatedLink = simulatedLink;
}

} // namespace Can
} // namespace Services
} // namespace SilKit

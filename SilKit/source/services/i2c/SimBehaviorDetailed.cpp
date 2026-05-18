// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "I2cController.hpp"
#include "SimBehaviorDetailed.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

SimBehaviorDetailed::SimBehaviorDetailed(Core::IParticipantInternal* participant, I2cController* controller,
                                         const Core::ServiceDescriptor& serviceDescriptor)
    : _participant{participant}
    , _parentServiceEndpoint{dynamic_cast<Core::IServiceEndpoint*>(controller)}
    , _parentServiceDescriptor{&serviceDescriptor}
    , _tracer{controller->GetTracer()}
{
}

template <typename MsgT>
void SimBehaviorDetailed::SendMsgImpl(MsgT&& msg)
{
    _participant->SendMsg(_parentServiceEndpoint, _simulatedLink.GetParticipantName(), std::forward<MsgT>(msg));
}

void SimBehaviorDetailed::SendMsg(WireI2cFrameEvent&& msg)
{
    _tracer->Trace(msg.direction, msg.timestamp, ToI2cFrameEvent(msg));
    SendMsgImpl(msg);
}

void SimBehaviorDetailed::SendMsg(WireI2cControllerConfig&& msg)
{
    SendMsgImpl(msg);
}

void SimBehaviorDetailed::OnControllerConfig(const Core::IServiceEndpoint* /*from*/,
                                             const WireI2cControllerConfig& /*config*/)
{
    // In detailed mode the network simulator manages address routing — no local registry needed.
}

auto SimBehaviorDetailed::AllowReception(const Core::IServiceEndpoint* from) const -> bool
{
    // Only accept messages from the network simulator
    const auto& fromDescr = from->GetServiceDescriptor();
    return _simulatedLink.GetParticipantName() == fromDescr.GetParticipantName()
           && _parentServiceDescriptor->GetServiceId() == fromDescr.GetServiceId();
}

void SimBehaviorDetailed::SetSimulatedLink(const Core::ServiceDescriptor& simulatedLink)
{
    _simulatedLink = simulatedLink;
}

} // namespace I2c
} // namespace Services
} // namespace SilKit

// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "LinEventProducer.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {
namespace Lin {

LinEventProducer::LinEventProducer(SimulatedNetworkRouter* simulatedNetworkRouter)
    : _simulatedNetworkRouter{simulatedNetworkRouter}
{
}


void LinEventProducer::Produce(const SilKit::Services::Lin::LinFrameStatusEvent& msg,
                               const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    SilKit::Services::Lin::LinTransmission wireMsg{};
    wireMsg.frame = msg.frame;
    wireMsg.status = msg.status;
    wireMsg.timestamp = msg.timestamp;
    _simulatedNetworkRouter->SendMsg(std::move(wireMsg), receivers);
}

void LinEventProducer::Produce(const SilKit::Services::Lin::LinSendFrameHeaderRequest& msg,
                               const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    _simulatedNetworkRouter->SendMsg(std::move(msg), receivers);
}

void LinEventProducer::Produce(const SilKit::Services::Lin::LinWakeupEvent& msg,
                               const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    SilKit::Services::Lin::LinWakeupPulse wireMsg{};
    wireMsg.direction = msg.direction;
    wireMsg.timestamp = msg.timestamp;
    _simulatedNetworkRouter->SendMsg(std::move(wireMsg), receivers);
}

} // namespace Lin
} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
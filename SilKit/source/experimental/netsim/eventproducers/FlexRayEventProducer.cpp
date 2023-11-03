// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "FlexRayEventProducer.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {
namespace Flexray {

FlexRayEventProducer::FlexRayEventProducer(SimulatedNetworkRouter* simulatedNetworkRouter)
    : _simulatedNetworkRouter{simulatedNetworkRouter}
{
}

void FlexRayEventProducer::Produce(
    const SilKit::Services::Flexray::FlexrayFrameEvent& msg,
    const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    auto wireMsg = SilKit::Services::Flexray::MakeWireFlexrayFrameEvent(msg);
    _simulatedNetworkRouter->SendMsg(std::move(wireMsg), receivers);
}

void FlexRayEventProducer::Produce(
    const SilKit::Services::Flexray::FlexrayFrameTransmitEvent& msg,
    const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    auto wireMsg = SilKit::Services::Flexray::MakeWireFlexrayFrameTransmitEvent(msg);
    _simulatedNetworkRouter->SendMsg(std::move(wireMsg), receivers);
}

void FlexRayEventProducer::Produce(
    const SilKit::Services::Flexray::FlexraySymbolEvent& msg,
    const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    _simulatedNetworkRouter->SendMsg(msg, receivers);
}

void FlexRayEventProducer::Produce(
    const SilKit::Services::Flexray::FlexraySymbolTransmitEvent& msg,
    const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    _simulatedNetworkRouter->SendMsg(msg, receivers);
}

void FlexRayEventProducer::Produce(
    const SilKit::Services::Flexray::FlexrayCycleStartEvent& msg,
    const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    _simulatedNetworkRouter->SendMsg(msg, receivers);
}

void FlexRayEventProducer::Produce(
    const SilKit::Services::Flexray::FlexrayPocStatusEvent& msg,
    const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    _simulatedNetworkRouter->SendMsg(msg, receivers);
}

} // namespace Flexray
} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
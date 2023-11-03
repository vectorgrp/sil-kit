// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "EthernetEventProducer.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {
namespace Ethernet {

EthernetEventProducer::EthernetEventProducer(SimulatedNetworkRouter* simulatedNetworkRouter)
    : _simulatedNetworkRouter{simulatedNetworkRouter}
{
}

void EthernetEventProducer::Produce(const SilKit::Services::Ethernet::EthernetFrameEvent& msg,
                                         const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    _simulatedNetworkRouter->SendMsg(SilKit::Services::Ethernet::MakeWireEthernetFrameEvent(msg), receivers);
}

void EthernetEventProducer::Produce(const SilKit::Services::Ethernet::EthernetFrameTransmitEvent& msg,
                                                 const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    _simulatedNetworkRouter->SendMsg(msg, receivers);
}

void EthernetEventProducer::Produce(const SilKit::Services::Ethernet::EthernetStateChangeEvent& msg,
                                               const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    SilKit::Services::Ethernet::EthernetStatus wireMsg{};

    // Transport the last used value that the receiving controller doesn't fire the corresponding handler
    wireMsg.bitrate = _ethernetBitrate; 
    _ethernetState = msg.state;

    wireMsg.state = msg.state;
    wireMsg.timestamp = msg.timestamp;
    _simulatedNetworkRouter->SendMsg(std::move(wireMsg), receivers);
}

void EthernetEventProducer::Produce(const SilKit::Services::Ethernet::EthernetBitrateChangeEvent& msg,
                                                    const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    SilKit::Services::Ethernet::EthernetStatus wireMsg{};

    // Transport the last used value that the receiving controller doesn't fire the corresponding handler
    wireMsg.state = _ethernetState;
    _ethernetBitrate = msg.bitrate;

    wireMsg.bitrate = msg.bitrate;
    wireMsg.timestamp = msg.timestamp;
    _simulatedNetworkRouter->SendMsg(std::move(wireMsg), receivers);
}

} // namespace Ethernet
} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
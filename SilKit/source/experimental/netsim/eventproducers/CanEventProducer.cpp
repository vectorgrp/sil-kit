// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "CanEventProducer.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {
namespace Can {

CanEventProducer::CanEventProducer(SimulatedNetworkRouter* simulatedNetworkRouter)
    : _simulatedNetworkRouter{simulatedNetworkRouter}
{
}

void CanEventProducer::Produce(const SilKit::Services::Can::CanFrameEvent& msg,
                                         const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    _simulatedNetworkRouter->SendMsg(SilKit::Services::Can::MakeWireCanFrameEvent(msg), receivers);
}

void CanEventProducer::Produce(const SilKit::Services::Can::CanFrameTransmitEvent& msg,
                                                 const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    _simulatedNetworkRouter->SendMsg(msg, receivers);
}

void CanEventProducer::Produce(const SilKit::Services::Can::CanStateChangeEvent& msg,
                                               const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    SilKit::Services::Can::CanControllerStatus controllerStatus{};
    
    // Transport the last used value that the receiving controller doesn't fire the corresponding handler
    controllerStatus.errorState = _errorState;
    _controllerState = msg.state;

    controllerStatus.controllerState = msg.state;
    controllerStatus.timestamp = msg.timestamp;
    _simulatedNetworkRouter->SendMsg(std::move(controllerStatus), receivers);
}

void CanEventProducer::Produce(const SilKit::Services::Can::CanErrorStateChangeEvent& msg,
                                                    const SilKit::Util::Span<const ControllerDescriptor>& receivers)
{
    SilKit::Services::Can::CanControllerStatus controllerStatus{};

    // Transport the last used value that the receiving controller doesn't fire the corresponding handler
    controllerStatus.controllerState = _controllerState; 
    _errorState = msg.errorState;

    controllerStatus.errorState = msg.errorState;
    controllerStatus.timestamp = msg.timestamp;
    _simulatedNetworkRouter->SendMsg(std::move(controllerStatus), receivers);
}

} // namespace Can
} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
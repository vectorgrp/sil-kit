// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <unordered_set>
#include <memory>
#include <functional>

#include "NetworkSimulatorDatatypes.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

//! Base class of a simulated controller.
class ISimulatedController
{
public:
    virtual ~ISimulatedController() = default;
};

//! Base class of an event producer.
class IEventProducer
{
public:
    virtual ~IEventProducer() = default;
};

//! Interface for a simulated network. 
//! An instance of a class inheriting from this interface is passed to the SIL Kit via \ref INetworkSimulator::SimulateNetwork. 
//! The interface functions are then invoked by the SIL Kit.
class ISimulatedNetwork
{
public:
    virtual ~ISimulatedNetwork() = default;

    /*! \brief Called when the API requires the implementation of a simulated controller. 
     *         This happens when a controller is created on a simulated network. 
     *         The network must be registered via \ref SilKit::Experimental::NetworkSimulation::INetworkSimulator::SimulateNetwork beforehand.
     *         After a simulated controller is provided, messages sent by the original controller will triggers message specific callbacks on the simulated controller.
     *  \param controllerDescriptor The identifier of a remote controller. 
     *         Used to address a specific controller as receiver when using the eventProducer.
     *  \return The implementation must return a valid pointer to a \ref SilKit::Experimental::NetworkSimulation::ISimulatedController.
     *          Note that this interface is abstract, the underlying class must inherit from the appropriate network-specific interface (e.g. \ref Can::ISimulatedCanController, \ref Flexray::ISimulatedFlexRayController, etc.).
     */
    virtual auto ProvideSimulatedController(ControllerDescriptor controllerDescriptor) -> ISimulatedController* = 0;

    /*! \brief Deregistration of a simulated controller.
     *         Called when the participant that owns the controller leaves the simulation.
     *  \param controllerDescriptor The identifier of a remote controller.
     */
    virtual void SimulatedControllerRemoved(ControllerDescriptor controllerDescriptor) = 0;
    
    /*! \brief Called once to provide an \ref SilKit::Experimental::NetworkSimulation::IEventProducer. 
     *         This happens when the first controller appears on a simulated network.
     *  \param eventProducer Used to generate events for a given group of receivers. 
               Note that this interface is abstract, the object must be cast to the specific interface for the underlying network type (\ref Can::ICanEventProducer, \ref Flexray::IFlexRayEventProducer, etc.).
     */
    virtual void SetEventProducer(std::unique_ptr<IEventProducer> eventProducer) = 0;
};

//! Network Simulator interface to register simulated networks and to start the network simulation.
//! A network simulator object can be obtained via \ref SilKit::Experimental::Participant::CreateNetworkSimulator.
//! Note that there can only be one network simulater per participant.
class INetworkSimulator
{
public:
    virtual ~INetworkSimulator() = default;

    /*! \brief Register a simulated network. 
     *         The network of the given name and type is subject to a detailed simulation whose logic is provided by the custom network simulator.
     *         Controllers with the same network name and type will no longer broadcast their outgoing messages to other controllers. 
     *         Instead, the traffic is routed to the network simulator participant and will arrive on a \ref ISimulatedController.
     *         Based on the incoming messages (e.g., a frame request), the simulation logic can be performed and outgoing events (e.g. acknowledgments, frames) can be produced. 
     *  \param networkName The name of the simulated network.
     *  \param networkType The type of the simulated network.
     *  \param simulatedNetwork Provided \ref ISimulatedNetwork object to manage a simulated network. 
               When a controller appears on a simulated network, the \ref ISimulatedNetwork, which was passed as argument to this method, is informed with a call to \ref ISimulatedNetwork::ProvideSimulatedController.
     *         There, a specific \ref ISimulatedController (e.g. \ref Can::ISimulatedCanController, \ref Flexray::ISimulatedFlexRayController, etc.) can be provided to receive the messages sent by the original controller.
     */
    virtual void SimulateNetwork(const std::string& networkName, SimulatedNetworkType networkType,
                                 std::unique_ptr<ISimulatedNetwork> simulatedNetwork) = 0;

    /*! \brief Start the network simulation of all previously registered networks.
     *         Simulated networks registered via INetworkSimulator::SimulateNetwork will be informed about corresponding controllers in the simulation with calls to \ref ISimulatedNetwork::ProvideSimulatedController.
     *         This holds true for remote controllers that have been created before the call to \ref INetworkSimulator::Start.
     *         Internally, remote controllers are informed that they are now part of a detailed network simulation and will route their messages to the network simulator.
     *         See the documentation for further information about the usage within the SIL Kit Lifecyle in order not to miss any messages on the network simulator.
     */        
    virtual void Start() = 0;
};

namespace Can {

/*! \brief API for simulated CAN controllers.
 *  If a new CAN controller is created on a simulated network, an implementation of this interface can be provided in \ref ISimulatedNetwork::ProvideSimulatedController.
 *  Messages generated by the original controller will be redirected to the simulated controller and trigger the On... callbacks.
 */
class ISimulatedCanController : public ISimulatedController
{
public:
    virtual ~ISimulatedCanController() = default;

    /*! \brief Incoming CAN controller message.
     *  Triggered when a CAN controller calls SilKit::Services::Can::ICanController::SetBaudRate().
     *  \param canConfigureBaudrate The configured baud rate of the controller.
     */
    virtual void OnSetBaudrate(const CanConfigureBaudrate& canConfigureBaudrate) = 0;

    /*! \brief Incoming CAN controller message.
     *  Triggered when a CAN controller calls \ref SilKit::Services::Can::ICanController::SendFrame().
     *  \param canFrameRequest The CAN frame requested to be sent.
     */
    virtual void OnFrameRequest(const CanFrameRequest& canFrameRequest) = 0;

    /*! \brief Incoming CAN controller message.
     *  Triggered when a CAN controller calls \ref SilKit::Services::Can::ICanController::Start(), \ref SilKit::Services::Can::ICanController::Stop(), \ref SilKit::Services::Can::ICanController::Sleep() or \ref SilKit::Services::Can::ICanController::Reset().
     *  \param canControllerMode The new controller state.
     */
    virtual void OnSetControllerMode(const CanControllerMode& canControllerMode) = 0;
};

//! Producer for CAN events.
//! Passed in \ref ISimulatedNetwork::SetEventProducer.
//! Messages can be produced for a given set of controllers, specified by a span of controller descriptors.
//! The individual controller descriptor of a remote controller is passed in \ref ISimulatedNetwork::ProvideSimulatedController.
class ICanEventProducer : public IEventProducer
{
public:
    virtual ~ICanEventProducer() = default;

    /*! \brief Produce a \ref SilKit::Services::Can::CanFrameEvent for a given set of receivers on this network.
     *  \param frameEvent The produced CAN event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Can::CanFrameEvent& frameEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Can::CanFrameTransmitEvent for a given set of receivers on this network.
     *  \param frameTransmitEvent The produced CAN event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Can::CanFrameTransmitEvent& frameTransmitEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Can::CanStateChangeEvent for a given set of receivers on this network.
     *  \param stateChangeEvent The produced CAN event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Can::CanStateChangeEvent& stateChangeEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Can::CanErrorStateChangeEvent for a given set of receivers on this network.
     *  \param errorStateChangeEvent The produced CAN event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Can::CanErrorStateChangeEvent& errorStateChangeEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;
};

} // namespace Can

namespace Flexray {

/*! \brief API for simulated FlexRay controllers.
 *  If a new FlexRay controller is created on a simulated network, an implementation of this interface can be provided in \ref ISimulatedNetwork::ProvideSimulatedController.
 *  Messages generated by the original controller will be redirected to the simulated controller and trigger the On... callbacks.
 */
class ISimulatedFlexRayController : public ISimulatedController
{
public:
    virtual ~ISimulatedFlexRayController() = default;

    /*! \brief Incoming FlexRay controller message.
     *  Triggered when a FlexRay controller calls SilKit::Services::Flexray::IFlexrayController::Run(), 
                                                  SilKit::Services::Flexray::IFlexrayController::DeferredHalt(), 
                                                  SilKit::Services::Flexray::IFlexrayController::Freeze(), 
                                                  SilKit::Services::Flexray::IFlexrayController::AllowColdstart(), 
                                                  SilKit::Services::Flexray::IFlexrayController::Wakeup() or
                                                  SilKit::Services::Flexray::IFlexrayController::AllSlots().
     *  \param flexrayHostCommand An identifier of the host command.
     */
    virtual void OnHostCommand(const FlexrayHostCommand& flexrayHostCommand) = 0;

    /*! \brief Incoming FlexRay controller message.
     *  Triggered when a FlexRay controller calls \ref SilKit::Services::Flexray::IFlexrayController::Configure.
     *  \param flexrayControllerConfig The FlexRay cluster parameters, node parameters and initial TX buffer configuration.
     */
    virtual void OnControllerConfig(const FlexrayControllerConfig& flexrayControllerConfig) = 0;

    /*! \brief Incoming FlexRay controller message.
     *  Triggered when a FlexRay controller calls \ref SilKit::Services::Flexray::IFlexrayController::ReconfigureTxBuffer.
     *  \param flexrayTxBufferConfigUpdate The index and the new configuration of a TX buffer .
     */
    virtual void OnTxBufferConfigUpdate(const FlexrayTxBufferConfigUpdate& flexrayTxBufferConfigUpdate) = 0;

    /*! \brief Incoming FlexRay controller message.
     *  Triggered when a FlexRay controller calls \ref SilKit::Services::Flexray::IFlexrayController::UpdateTxBuffer.
     *  \param flexrayTxBufferUpdate The index and the new payload of a TX buffer.
     */
    virtual void OnTxBufferUpdate(const FlexrayTxBufferUpdate& flexrayTxBufferUpdate) = 0;
};

//! Producer for FlexRay events.
//! Passed in \ref ISimulatedNetwork::SetEventProducer.
//! Messages can be produced for a given set of controllers, specified by a span of controller descriptors.
//! The individual controller descriptor of a remote controller is passed in \ref ISimulatedNetwork::ProvideSimulatedController.
class IFlexRayEventProducer : public IEventProducer
{
public:
    virtual ~IFlexRayEventProducer() = default;

    /*! \brief Produce a \ref SilKit::Services::Flexray::FlexrayFrameEvent for a given set of receivers on this network.
     *  \param frameEvent The produced FlexRay event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Flexray::FlexrayFrameEvent& frameEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Flexray::FlexrayFrameTransmitEvent for a given set of receivers on this network.
     *  \param frameTransmitEvent The produced FlexRay event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Flexray::FlexrayFrameTransmitEvent& frameTransmitEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Flexray::FlexraySymbolEvent for a given set of receivers on this network.
     *  \param symbolEvent The produced FlexRay event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Flexray::FlexraySymbolEvent& symbolEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Flexray::FlexraySymbolTransmitEvent for a given set of receivers on this network.
     *  \param symbolTransmitEvent The produced FlexRay event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Flexray::FlexraySymbolTransmitEvent& symbolTransmitEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Flexray::FlexrayCycleStartEvent for a given set of receivers on this network.
     *  \param cycleStartEvent The produced FlexRay event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Flexray::FlexrayCycleStartEvent& cycleStartEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Flexray::FlexrayPocStatusEvent for a given set of receivers on this network.
     *  \param pocStatusEvent The produced FlexRay event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Flexray::FlexrayPocStatusEvent& pocStatusEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;
};

} // namespace Flexray

namespace Ethernet {

/*! \brief API for simulated Ethernet controllers.
 *  If a new Ethernet controller is created on a simulated network, an implementation of this interface can be provided in \ref ISimulatedNetwork::ProvideSimulatedController.
 *  Messages generated by the original controller will be redirected to the simulated controller and trigger the On... callbacks.
 */
class ISimulatedEthernetController : public ISimulatedController
{
public:
    virtual ~ISimulatedEthernetController() = default;

    /*! \brief Incoming Ethernet controller message.
     *  Triggered when a Ethernet controller calls \ref SilKit::Services::Ethernet::IEthernetController::SendFrame.
     *  \param ethernetFrameRequest The Ethernet frame requested to be sent.
     */
    virtual void OnFrameRequest(const EthernetFrameRequest& ethernetFrameRequest) = 0;

    /*! \brief Incoming Ethernet controller message.
     *  Triggered when a Ethernet controller calls \ref SilKit::Services::Ethernet::IEthernetController::Activate or
     .                                             \ref SilKit::Services::Ethernet::IEthernetController::Deactivate 
     *  \param ethernetControllerMode The current Ethernet controller mode.
     */
    virtual void OnSetControllerMode(const EthernetControllerMode& ethernetControllerMode) = 0;
};

//! Producer for Ethernet events.
//! Passed in \ref ISimulatedNetwork::SetEventProducer.
//! Messages can be produced for a given set of controllers, specified by a span of controller descriptors.
//! The individual controller descriptor of a remote controller is passed in \ref ISimulatedNetwork::ProvideSimulatedController.
class IEthernetEventProducer : public IEventProducer
{
public:
    virtual ~IEthernetEventProducer() = default;

    /*! \brief Produce a \ref SilKit::Services::Ethernet::EthernetFrameEvent for a given set of receivers on this network.
     *  \param frameEvent The produced Ethernet event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Ethernet::EthernetFrameEvent& frameEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Ethernet::EthernetFrameTransmitEvent for a given set of receivers on this network.
     *  \param frameTransmitEvent The produced Ethernet event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Ethernet::EthernetFrameTransmitEvent& frameTransmitEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Ethernet::EthernetStateChangeEvent for a given set of receivers on this network.
     *  \param stateChangeEvent The produced Ethernet event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Ethernet::EthernetStateChangeEvent& stateChangeEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Ethernet::EthernetBitrateChangeEvent for a given set of receivers on this network.
     *  \param bitrateChangeEvent The produced Ethernet event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Ethernet::EthernetBitrateChangeEvent& bitrateChangeEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;
};

} // namespace Ethernet

namespace Lin {

/*! \brief API for simulated LIN controllers.
 *  If a new LIN controller is created on a simulated network, an implementation of this interface can be provided in \ref ISimulatedNetwork::ProvideSimulatedController.
 *  Messages generated by the original controller will be redirected to the simulated controller and trigger the On... callbacks.
 */
class ISimulatedLinController : public ISimulatedController
{
public:
    virtual ~ISimulatedLinController() = default;

    /*! \brief Incoming LIN controller message.
     *  Triggered when a LIN controller calls \ref SilKit::Services::Lin::ILinController::SendFrame.
     *  \param linFrameRequest The LIN frame requested to be sent.
     */
    virtual void OnFrameRequest(const LinFrameRequest& linFrameRequest) = 0;

    /*! \brief Incoming LIN controller message.
     *  Triggered when a LIN controller calls \ref SilKit::Services::Lin::ILinController::SendFrameHeader.
     *  \param linFrameHeaderRequest A LIN frame header.
     */
    virtual void OnFrameHeaderRequest(const LinFrameHeaderRequest& linFrameHeaderRequest) = 0;

    /*! \brief Incoming LIN controller message.
     *  Triggered when a LIN controller calls \ref SilKit::Services::Lin::ILinController::Wakeup.
     *  \param linWakeupPulse The Wakeup pulse without any further data except the event timestamp.
     */
    virtual void OnWakeupPulse(const LinWakeupPulse& linWakeupPulse) = 0;

    /*! \brief Incoming LIN controller message.
     *  Triggered when a LIN controller calls \ref SilKit::Services::Lin::ILinController::Init.
     *  \param linControllerConfig The LIN controller configuration and frame response data.
     */
    virtual void OnControllerConfig(const LinControllerConfig& linControllerConfig) = 0;

    /*! \brief Incoming LIN controller message.
     *  Triggered when a LIN controller calls \ref SilKit::Services::Lin::ILinController::UpdateTxBuffer
     *  or \ref SilKit::Services::Lin::ILinController::SendFrame.
     *  \param linFrameResponseUpdate An update for the frame response data.
     */
    virtual void OnFrameResponseUpdate(const LinFrameResponseUpdate& linFrameResponseUpdate) = 0;

    /*! \brief Incoming LIN controller message.
     *  Triggered when a LIN controller calls \ref SilKit::Services::Lin::ILinController::Wakeup,
     *                                        \ref SilKit::Services::Lin::ILinController::WakeupInternal,
     *                                        \ref SilKit::Services::Lin::ILinController::GoToSleep or
     *                                        \ref SilKit::Services::Lin::ILinController::GoToSleepInternal
     *  \param linControllerStatusUpdate The new LIN controller status.
     */
    virtual void OnControllerStatusUpdate(const LinControllerStatusUpdate& linControllerStatusUpdate) = 0;
};

//! Producer for LIN events.
//! Passed in \ref ISimulatedNetwork::SetEventProducer.
//! Messages can be produced for a given set of controllers, specified by a span of controller descriptors.
//! The individual controller descriptor of a remote controller is passed in \ref ISimulatedNetwork::ProvideSimulatedController.
class ILinEventProducer : public IEventProducer
{
public:
    virtual ~ILinEventProducer() = default;

    /*! \brief Produce a \ref SilKit::Services::Lin::LinFrameStatusEvent for a given set of receivers on this network.
     *  \param frameStatusEvent The produced Lin event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Lin::LinFrameStatusEvent& frameStatusEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Lin::LinSendFrameHeaderRequest for a given set of receivers on this network.
     *  \param sendFrameHeaderRequest The produced Lin event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Lin::LinSendFrameHeaderRequest& sendFrameHeaderRequest,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;

    /*! \brief Produce a \ref SilKit::Services::Lin::LinWakeupEvent for a given set of receivers on this network.
     *  \param wakeupEvent The produced Lin event.
     *  \param receivers The recipients of the event as a span of controller descriptors.
     */
    virtual void Produce(const SilKit::Services::Lin::LinWakeupEvent& wakeupEvent,
                         const SilKit::Util::Span<const ControllerDescriptor>& receivers) = 0;
};

} // namespace Lin

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
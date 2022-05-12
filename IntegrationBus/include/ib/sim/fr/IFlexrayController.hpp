// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <functional>

#include "FlexrayDatatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief Abstract FlexRay Controller API to be used by vECUs
 */
class IFlexrayController
{
public:
    /*! \brief Generic FleyRay callback method
    */
    template<typename MsgT>
    using CallbackT = std::function<void(IFlexrayController* controller, const MsgT& msg)>;

    /*! Callback type to indicate that a FlexRay message has been received.
     *  Cf. \ref AddFrameHandler();
     */
    using FrameHandler = CallbackT<FlexrayFrameEvent>;

    /*! Callback type to indicate that a FlexrayFrameTransmitEvent has been received.
     *  Cf. \ref AddFrameTransmitHandler();
     */
    using FrameTransmitHandler = CallbackT<FlexrayFrameTransmitEvent>;

    /*! Callback type to indicate that a wakeup has been received.
     *   Should be answered by a call to Run(). Cf. \ref AddWakeupHandler();
     */
    using WakeupHandler = CallbackT<FlexrayWakeupEvent>;

    //! \brief Callback type to indicate that the POC status (including state variables, modes and error codes) has changed.
    using PocStatusHandler = CallbackT<FlexrayPocStatusEvent>;

    /*! Callback type to indicate that the controller has received a symbol.
     *  Cf. \ref AddSymbolHandler();
     */
    using SymbolHandler = CallbackT<FlexraySymbolEvent>;

    /*! Callback type to indicate that the controller has sent a symbol.
     *  Cf. \ref AddSymbolTransmitHandler();
     */
    using SymbolTransmitHandler = CallbackT<FlexraySymbolTransmitEvent>;

    /*! Callback type to indicate that a new FlexRay cycle did start.
     *  Cf. \ref AddCycleStartHandler();
     *
     *  NB: Only supported in VIBE simulation.
     */
    using CycleStartHandler = CallbackT<FlexrayCycleStartEvent>;

public:
    virtual ~IFlexrayController() = default;

    //! \brief Configure the controller and switch to ::Ready state
    virtual void Configure(const FlexrayControllerConfig& config) = 0;

    //! \brief Reconfigure a TX Buffer that was previously setup with IFlexrayController::Configure(const FlexrayControllerConfig&)
    virtual void ReconfigureTxBuffer(uint16_t txBufferIdx, const FlexrayTxBufferConfig& config) = 0;

    /*! \brief Update the content of a previously configured TX buffer.
     *
     * Due to the fixed and repetitive cycle of FlexRay, the behavior of UpdateTxBuffer is
     * quite different when using a detailed Network Simulator or not.
     *
     * If a Network Simulator is used, a FlexRay message will be sent at the time matching to
     * the configured Slot ID. If the buffer was configured with FlexrayTransmissionMode::SingleShot,
     * the content is sent exactly once. If it is configured as FlexrayTransmissionMode::Continuous,
     * the content is sent repeatedly according to the offset and repetition configuration.
     *
     * Without a Network Simulator, a FlexRay message will be sent immediately and only once.
     * I.e., the configuration according to cycle, repetition, and transmission mode is
     * ignored. In particular, even with FlexrayTransmissionMode::Continuous, the message will be
     * sent only once.
     *
     *  \see IFlexrayController::Configure(const FlexrayControllerConfig&)
     */
    virtual void UpdateTxBuffer(const FlexrayTxBufferUpdate& update) = 0;

    /*! \brief Send the FlexrayChiCommand::RUN
    *
    *  NB: Only supported in VIBE simulation. In simple simulation, this command
    *  simply sends a dummy symbol to emulate the control flow at startup amd
    *  calls the ControllerStateHandler with FlexrayPocState::NormalActive.
    */
    virtual void Run() = 0;

    /*! \brief Send the FlexrayChiCommand::DEFERRED_HALT
    *
    *  NB: Only supported in VIBE simulation.
    */
    virtual void DeferredHalt() = 0;

    /*! \brief Send the FlexrayChiCommand::FREEZE
    *
    *  NB: Only supported in VIBE simulation.
    */
    virtual void Freeze() = 0;

    /*! \brief Send the FlexrayChiCommand::ALLOW_COLDSTART
    *
    *  NB: Only supported in VIBE simulation.
    */
    virtual void AllowColdstart() = 0;

    /*! \brief Send the FlexrayChiCommand::ALL_SLOTS
    *
    *  NB: Only supported in VIBE simulation.
    */
    virtual void AllSlots() = 0;

    //! \brief Send the FlexrayChiCommand::WAKEUP
    virtual void Wakeup() = 0;

    //! \brief Receive a FlexRay message from a different controller.
    virtual void AddFrameHandler(FrameHandler handler) = 0;
    //! \brief Notification that a FlexRay message has been successfully sent.
    virtual void AddFrameTransmitHandler(FrameTransmitHandler handler) = 0;
    //! \brief Notification that a wakeup has been received.
    virtual void AddWakeupHandler(WakeupHandler handler) = 0;
    //! \brief Notification that the POC status has changed.
    virtual void AddPocStatusHandler(PocStatusHandler handler) = 0;


    /*! \brief Notification that the controller has received a symbol.
    *
    * This callback is primarily intended for tracing. There is no need to react on it.
    * The symbols relevant for interaction trigger also an additional callback,
    * e.g., \ref WakeupHandler.
    */
    virtual void AddSymbolHandler(SymbolHandler handler) = 0;

    /*! \brief Notification that the controller has sent a symbol.
    *
    * This callback is primarily intended for tracing. There is no need to react on it.
    * Currently, the following SymbolPatterns can occur:
    *  - Wakeup() will cause sending the FlexraySymbolPattern::Wus, if the bus is idle.
    *  - Run() will cause the transmission of FlexraySymbolPattern::CasMts if configured to coldstart the bus.
    */
    virtual void AddSymbolTransmitHandler(SymbolTransmitHandler handler) = 0;

    /*! \brief Notification that a new FlexRay cycle did start.
    *
    *  NB: Only supported in VIBE simulation.
    */
    virtual void AddCycleStartHandler(CycleStartHandler handler) = 0;
};

} // namespace fr
} // SimModels
} // namespace ib

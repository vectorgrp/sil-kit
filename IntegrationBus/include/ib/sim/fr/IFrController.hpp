// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <functional>

#include "FrDatatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief Abstract FlexRay Controller API to be used by vECUs
 */
class IFrController
{
public:
    /*! \brief Generic FleyRay callback method
    */
    template<typename MsgT>
    using CallbackT = std::function<void(IFrController* controller, const MsgT& msg)>;

    /*! Callback type to indicate that a FlexRay message has been received.
    *  Cf. \ref RegisterMessageHandler();
    */
    using MessageHandler = CallbackT<FrMessage>;

    /*! Callback type to indicate that a FrMessageAck has been received.
    *  Cf. \ref RegisterMessageAckHandler();
    */
    using MessageAckHandler = CallbackT<FrMessageAck>;

    /*! Callback type to indicate that a wakeup has been received.
    *   Should be answered by a call to Run(). Cf. \ref RegisterWakeupHandler();
    */
    using WakeupHandler = CallbackT<FrSymbol>;

    /*! Callback type to indicate that the POC state has changed.
    *   Cf. \ref RegisterControllerStatusHandler();
    * \deprecated the ControllerStatus is deprecated in favor of the PocStatus.
    */
    using ControllerStatusHandler = CallbackT<ControllerStatus>;

    /*! Callback type to indicate that the POC status (including state variables, modes and error codes) has changed.
    * 
    */
    using PocStatusHandler = CallbackT<PocStatus>;

    /*! Callback type to indicate that the controller has received a symbol.
     *  Cf. \ref RegisterSymbolHandler();
     */
    using SymbolHandler = CallbackT<FrSymbol>;

    /*! Callback type to indicate that the controller has sent a symbol.
     *  Cf. \ref RegisterSymbolAckHandler();
     */
    using SymbolAckHandler = CallbackT<FrSymbolAck>;

    /*! Callback type to indicate that a new FlexRay cycle did start.
     *  Cf. \ref RegisterCycleStartHandler();
     *
     *  NB: Only supported in VIBE simulation.
     */
    using CycleStartHandler = CallbackT<CycleStart>;

public:
    virtual ~IFrController() = default;

    //! \brief Configure the controller and switch to ::Ready state
    virtual void Configure(const ControllerConfig& config) = 0;

    //! \brief Reconfigure a TX Buffer that was previously setup with IFrController::Configure(const ControllerConfig&)
    virtual void ReconfigureTxBuffer(uint16_t txBufferIdx, const TxBufferConfig& config) = 0;

    /*! \brief Update the content of a previously configured TX buffer.
     *
     * Due to the fixed and repetitive cycle of FlexRay, the behavior of UpdateTxBuffer is
     * quite different when using a detailed Network Simulator or not.
     *
     * If a Network Simulator is used, a FlexRay message will be sent at the time matching to
     * the configured Slot ID. If the buffer was configured with TransmissionMode::SingleShot,
     * the content is sent exactly once. If it is configured as TransmissionMode::Continuous,
     * the content is sent repeatedly according to the offset and repetition configuration.
     *
     * Without a Network Simulator, a FlexRay message will be sent immediately and only once.
     * I.e., the configuration according to cycle, repetition, and transmission mode is
     * ignored. In particular, even with TransmissionMode::Continuous, the message will be
     * sent only once.
     *
     *  \see IFrController::Configure(const ControllerConfig&)
     */
    virtual void UpdateTxBuffer(const TxBufferUpdate& update) = 0;

    /*! \brief Send the ChiCommand::RUN
    *
    *  NB: Only supported in VIBE simulation. In simple simulation, this command
    *  simply sends a dummy symbol to emulate the control flow at startup amd
    *  calls the ControllerStateHandler with PocState::NormalActive.
    */
    virtual void Run() = 0;

    /*! \brief Send the ChiCommand::DEFERRED_HALT
    *
    *  NB: Only supported in VIBE simulation.
    */
    virtual void DeferredHalt() = 0;

    /*! \brief Send the ChiCommand::FREEZE
    *
    *  NB: Only supported in VIBE simulation.
    */
    virtual void Freeze() = 0;

    /*! \brief Send the ChiCommand::ALLOW_COLDSTART
    *
    *  NB: Only supported in VIBE simulation.
    */
    virtual void AllowColdstart() = 0;

    /*! \brief Send the ChiCommand::ALL_SLOTS
    *
    *  NB: Only supported in VIBE simulation.
    */
    virtual void AllSlots() = 0;

    //! \brief Send the ChiCommand::WAKEUP
    virtual void Wakeup() = 0;

    //! \brief Receive a FlexRay message from a different controller.
    virtual void RegisterMessageHandler(MessageHandler handler) = 0;
    //! \brief Notification that a FlexRay message has been successfully sent.
    virtual void RegisterMessageAckHandler(MessageAckHandler handler) = 0;
    //! \brief Notification that a wakeup has been received.
    virtual void RegisterWakeupHandler(WakeupHandler handler) = 0;
    //! \brief Notification that the POC state has changed (deprecated API, cf.
    //         RegisterPocStatusHandler).
    virtual void RegisterControllerStatusHandler(ControllerStatusHandler handler) = 0;
    //! \brief Notification that the POC status has changed.
    virtual void RegisterPocStatusHandler(PocStatusHandler handler) = 0;


    /*! \brief Notification that the controller has received a symbol.
    *
    * This callback is primarily intended for tracing. There is no need to react on it.
    * The symbols relevant for interaction trigger also an additional callback,
    * e.g., \ref WakeupHandler.
    */
    virtual void RegisterSymbolHandler(SymbolHandler handler) = 0;

    /*! \brief Notification that the controller has sent a symbol.
    *
    * This callback is primarily intended for tracing. There is no need to react on it.
    * Currently, the following SymbolPatterns can occur:
    *  - Wakeup() will cause sending the SymbolPattern::Wus, if the bus is idle.
    *  - Run() will cause the transmission of SymbolPattern::CasMts if configured to coldstart the bus.
    */
    virtual void RegisterSymbolAckHandler(SymbolAckHandler handler) = 0;

    /*! \brief Notification that a new FlexRay cycle did start.
    *
    *  NB: Only supported in VIBE simulation.
    */
    virtual void RegisterCycleStartHandler(CycleStartHandler handler) = 0;
};

} // namespace fr
} // SimModels
} // namespace ib

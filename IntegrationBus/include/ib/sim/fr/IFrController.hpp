// Copyright (c)  Vector Informatik GmbH. All rights reserved.

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
    template<typename MsgT>
    using CallbackT = std::function<void(IFrController* controller, const MsgT& msg)>;

    //! \brief Receive a FlexRay message from a different controller.
    using MessageHandler = CallbackT<FrMessage>;
    //! \brief Notification that a FlexRay message has been successfully sent.
    using MessageAckHandler = CallbackT<FrMessageAck>;
    //! \brief Notification that a wakeup has been received.
    //! Should be answered by a call to Run().
    using WakeupHandler = CallbackT<FrSymbol>;
    //! \brief Notification that the POC state has changed.
    using ControllerStatusHandler = CallbackT<ControllerStatus>;

    /*! \brief Notification that the controller has received a symbol.
     *
     * This callback is primarily intended for tracing. There is no need to react on it.
     * The symbols relevant for interaction trigger also an additional callback,
     * e.g., WakeupHandler.
     */
    using SymbolHandler = CallbackT<FrSymbol>;

    /*! \brief Notification that the controller has sent a symbol.
     *
     * This callback is primarily intended for tracing. There is no need to react on it.
     * Currently, the following SymbolPatterns can occur:
     *  - Wakeup() will cause sending the SymbolPattern::Wus, if the bus is idle
     *  - Run() will cause the transmission of CasMts if configured to coldstart the bus.
     */
    using SymbolAckHandler = CallbackT<FrSymbolAck>;

public:
    virtual ~IFrController() = default;

    //! \brief Configure the controller and switch to ready state
    virtual void Configure(const ControllerConfig& config) = 0;

    /*! \brief Update the content of a previously configured TX buffer.
     *
     * Due to the fixed and repetitive cycle of FlexRay, the behavior of UpdateTxBuffer is
     * quite different when using a detailed Network Simulator or not.
     *
     * If a Network Simulator is used, a FlexRay message will be sent at the time matching to
     * the configured Slot ID. If the buffer was configured with
     * TransmissionMode::SingleShot, the content is sent exactly once. If it is configured
     * as TransmissionMode::Continuous, the content is sent repeatedly according to the
     * offset and repetition configuration.
     *
     * Without a Network Simulator, a FlexRay message will be sent immediatly and only once.
     * I.e., the configuration according to cycle, repetition, and transmissionmode is
     * ignored. In particular, even with TransmissionMode::Continuous, the message will be
     * sent only once.
     *
     *  \see IFrController::Configure(const ControllerConfig&)
     */
    virtual void UpdateTxBuffer(const TxBufferUpdate& update) = 0;

    //! \brief Send the CHI command RUN
    virtual void Run() = 0;

    //! \brief Send the CHI command DEFERRED_HALT
    virtual void DeferredHalt() = 0;

    //! \brief Send the CHI command FREEZE
    virtual void Freeze() = 0;

    //! \brief Send the CHI command ALLOW_COLDSTART
    virtual void AllowColdstart() = 0;

    //! \brief Send the CHI command ALL_SLOTS
    virtual void AllSlots() = 0;

    //! \brief Send the CHI command WAKEUP
    virtual void Wakeup() = 0;

    //! \brief Receive a FlexRay message from a different controller.
    virtual void RegisterMessageHandler(MessageHandler handler) = 0;
    //! \brief Notification that a FlexRay message has been successfully sent.
    virtual void RegisterMessageAckHandler(MessageAckHandler handler) = 0;
    //! \brief Notification that a wakeup has been received
    virtual void RegisterWakeupHandler(WakeupHandler handler) = 0;
    //! \brief Notification that the POC state has changed.
    virtual void RegisterControllerStatusHandler(ControllerStatusHandler handler) = 0;
    //! \brief Notification that the controller has received a symbol.
    virtual void RegisterSymbolHandler(SymbolHandler handler) = 0;
    //! \brief Notification that the controller has sent a symbol.
    virtual void RegisterSymbolAckHandler(SymbolAckHandler handler) = 0;
};

} // namespace fr
} // SimModels
} // namespace ib

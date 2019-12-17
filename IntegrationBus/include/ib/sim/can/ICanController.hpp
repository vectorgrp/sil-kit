// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

#include "CanDatatypes.hpp"

namespace ib {
namespace sim {
namespace can {

/*! \brief Abstract CAN Controller API to be used by vECUs
 */
class ICanController
{
public:
    /*! \brief Generic CAN callback method
    */
    template<typename MsgT>
    using CallbackT = std::function<void(ICanController* controller, const MsgT& msg)>;

    /*! Callback type to indicate that a CanMessage has been received.
    *  Cf., \ref RegisterReceiveMessageHandler(ReceiveMessageHandler);
    */
    using ReceiveMessageHandler    = CallbackT<CanMessage>;

    /*! Callback type to indicate that the ::CanControllerState has changed.
    *  Cf., \ref RegisterStateChangedHandler(StateChangedHandler);
    */
    using StateChangedHandler      = CallbackT<CanControllerState>;

    /*! Callback type to indicate that the controller ::CanErrorState has changed.
    *  Cf., \ref RegisterErrorStateChangedHandler(ErrorStateChangedHandler);
    */
    using ErrorStateChangedHandler = CallbackT<CanErrorState>;

    /*! Callback type to indicate that a CanTransmitAcknowledge has been received.
    *  Cf., \ref RegisterTransmitStatusHandler(MessageStatusHandler);
    */
    using MessageStatusHandler     = CallbackT<CanTransmitAcknowledge>;

public:
    virtual ~ICanController() = default;

    /*! \brief Configure the baudrate of the controller
     *
     * \param rate Baud rate for regular (non FD) CAN messages given
     * in bps; valid range: 0 to 2'000'000
     *
     * \param fdRate Baud rate for CAN FD messages given in bps; valid
     * range: 0 to 16'000'000
     *
     * NB: In VIBE simulation, the baud rate is used to calculate
     * transmission delays of CAN messages and to determine proper
     * configuration and interoperation of the connected controllers.
     *
     * NB: The baud rate has no effect in simple simulation.
     */
    virtual void SetBaudRate(uint32_t rate, uint32_t fdRate) = 0;

    /*! \brief Reset the CAN controller
     *
     * Resets the controller's Transmit Error Count (TEC) and the
     * Receive Error Count (REC). Furthermore, sets the
     * CAN controller state to CanControllerState::Uninit and the
     * controller's error state to CanErrorState::NotAvailable.
     *
     * NB: Only supported in VIBE simulation, the command is ignored
     * in simple simulation.
     *
     * \ref Start(), \ref Stop(), \ref Sleep()
     */
    virtual void Reset() = 0;

    /*! \brief Start the CAN controller
     *
     * NB: Only supported in VIBE simulation, the command is ignored
     * in simple simulation.
     *
     * \ref Reset(), \ref Stop(), \ref Sleep()
     */
    virtual void Start() = 0;

    /*! \brief Stop the CAN controller
     *
     * NB: Only supported in VIBE simulation, the command is ignored
     * in simple simulation.
     *
     * \ref Reset(), \ref Start(), \ref Sleep()
     */
    virtual void Stop() = 0;

    /*! \brief Put the CAN controller in sleep mode
     *
     * NB: Only supported in VIBE simulation, the command is ignored
     * in simple simulation.
     *
     * \ref Reset(), Start(), \ref Stop()
     */
    virtual void Sleep() = 0;

    /*! \brief Request the transmission of a CanMessage
     *
     * NB: In VIBE simulation, the CanMessage must provide a valid CAN
     * ID and valid flags. The data length code is optional and is
     * automatically derived by the VIBE CAN simulator based on the
     * provided flags and the length of the dataField. The controller
     * must be in the Started state to transmit and receive messages.
     *
     * NB: In simple simulation, the requirements for VIBE simulation
     * are not enforced. I.e., CanMessages are distributed to
     * connected controllers regardless of the content and controller
     * states are not checked.
     *
     * \return Unique TX identifier to relate the request to following
     * CanTransmitAcknowledge messages.
     */
    virtual auto SendMessage(const CanMessage& msg) -> CanTxId = 0;

    /*! \brief Request the transmission of a CanMessage
     *
     * NB: In VIBE simulation, the CanMessage must provide a valid CAN
     * ID and valid flags. The data length code is optional and is
     * automatically derived by the VIBE CAN simulator based on the
     * provided flags and the length of the dataField. The controller
     * must be in the Started state to transmit and receive messages.
     *
     * NB: In simple simulation, the requirements for VIBE simulation
     * are not enforced. I.e., CanMessages are distributed to
     * connected controllers regardless of the content and controller
     * states are not checked.
     *
     * \return Unique TX identifier to relate the request to following
     * CanTransmitAcknowledge messages.
     */
    virtual auto SendMessage(CanMessage&& msg) -> CanTxId = 0;

    /*! \brief Register a callback for CAN message reception
     *
     * The registered handler is called when the controller receives a
     * new CanMessage.
     */
    virtual void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) = 0;

    /*! \brief Register a callback for controller state changes
     *
     * The registered handler is called when the ::CanControllerState of
     * the controller changes. E.g., after starting the controller, the
     * state changes from CanControllerState::Uninit to
     * CanControllerState::Started.
     * 
     * NB: Only supported in VIBE simulation. In simple simulation,
     * the handler is never called.
     */
    virtual void RegisterStateChangedHandler(StateChangedHandler handler) = 0;

    /*! \brief Register a callback for changes of the controller's error state
     *
     * The registered handler is called when the ::CanErrorState of the
     * controller changes. During normal operation, the controller
     * should be in state CanErrorState::ErrorActive. The states correspond
     * to the error state handling protocol of the CAN specification.
     *
     * NB: Only supported in VIBE simulation. In simple simulation,
     * the handler is never called.
     */
    virtual void RegisterErrorStateChangedHandler(ErrorStateChangedHandler handler) = 0;

    /*! \brief Register a callback for the TX status of sent CAN messages
     *
     * The registered handler is called when a CAN message was
     * successfully transmitted on the bus or when an error occurred.
     *
     * NB: Full support in VIBE simulation. In simple simulation, all
     * messages are automatically positively acknowledged.
     */
    virtual void RegisterTransmitStatusHandler(MessageStatusHandler handler) = 0;
};

} // namespace can
} // namespace sim
} // namespace ib

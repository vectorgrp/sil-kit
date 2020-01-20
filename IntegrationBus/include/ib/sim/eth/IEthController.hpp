// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

#include "EthDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

/*! \brief Abstract Ethernet Controller API to be used by vECUs
 */
class IEthController
{
public:
    /*! \brief Generic Ethernet callback method
    */
    template<typename MsgT>
    using CallbackT = std::function<void(IEthController* controller, const MsgT& msg)>;

    /*! Callback type to indicate that an EthMessage has been received.
    *  Cf. \ref RegisterReceiveMessageHandler(ReceiveMessageHandler);
    */
    using ReceiveMessageHandler = CallbackT<EthMessage>;

    /*! Callback type to indicate that an EthTransmitAcknowledge has been received.
    *  Cf. \ref RegisterMessageAckHandler(MessageAckHandler);
    */
    using MessageAckHandler     = CallbackT<EthTransmitAcknowledge>;

    /*! Callback type to indicate that the ::EthState has changed.
    *  Cf. \ref RegisterStateChangedHandler(StateChangedHandler);
    */
    using StateChangedHandler   = CallbackT<EthState>;

    /*! Callback type to indicate that the link bit rate has changed.
    *  Cf. \ref RegisterBitRateChangedHandler(BitRateChangedHandler);
    */
    using BitRateChangedHandler = CallbackT<uint32_t>;

public:
    virtual ~IEthController() = default;

    /*! \brief Activates the Ethernet controller
     *
     * Upon activation of the controller, the controller attempts to
     * establish a link. Messages can only be sent once the link has
     * been successfully established,
     * cf. RegisterStateChangedHandler() and RegisterBitRateChangedHandler().
     *
     * NB: Only supported in VIBE simulation! In simple simulation,
     * messages can be sent without need to call Activate()
     */
    virtual void Activate() = 0;

    /*! \brief Deactivate the Ethernet controller
     *
     * Deactivate the controller and shut down the link. The
     * controller will no longer receive messages, and it cannot send
     * messages anymore.
     *
     * NB: Only supported in VIBE simulation! In simple simulation,
     * Deactivate() has no effects and messages can still be sent.
     */
    virtual void Deactivate() = 0;

    /*! \brief Send an Ethernet message
     *
     * NB: In VIBE simulation, requires previous activation of the
     * controller and a successfully established link. Also, the
     * entire EthFrame must be valid, e.g., destination and source MAC
     * addresses must be valid, ether type and vlan tags must be
     * correct, payload size must be valid.
     *
     * These requirements for VIBE simulation are not enforced in
     * simple simulation. In this case, the message is simply passed
     * on to all connected controllers without performing any check.
     *
     * \return Unique transmit id to identify corresponding
     * acknowledgement.
     */
    virtual auto SendMessage(EthMessage msg) -> EthTxId = 0;

    /*! \brief Register a callback for Ethernet message reception
     *
     * The handler is called when the controller receives a new
     * Ethernet message.
     */
    virtual void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) = 0;

    /*! \brief Register a callback for Ethernet transmit acknowledgements
     *
     * The handler is called when a previously sent message was
     * successfully transmitted or when the transmission has
     * failed. The original message is identified by the transmitId.
     *
     * NB: Full support in VIBE Ethernet simulation. In simple
     * simulation, all messages are immediately positively
     * acknowledged by a receiving controller.
     */
    virtual void RegisterMessageAckHandler(MessageAckHandler handler) = 0;

    /*! \brief Register a callback for changes of the controller state
     *
     * The handler is called when the state of the controller
     * changes. E.g., a call to Activate() causes the controller to
     * change from state ::Inactive to ::LinkDown. Later, when the link
     * has been established, the state changes again from ::LinkDown to
     * ::LinkUp. Similarly, the status changes back to ::Inactive upon a
     * call to Deactivate().
     *
     * NB: Only supported in VIBE Ethernet simulation.
     */
    virtual void RegisterStateChangedHandler(StateChangedHandler handler) = 0;

    /*! \brief Register a callback for changes of the link bit rate
     *
     * The handler is called when the bit rate of the connected link
     * changes. This is typically the case when a link was
     * successfully established, or the controller was deactivated.
     *
     * NB: Only supported in VIBE Ethernet simulation.
     */
    virtual void RegisterBitRateChangedHandler(BitRateChangedHandler handler) = 0;
};

} // namespace eth
} // namespace sim
} // namespace ib

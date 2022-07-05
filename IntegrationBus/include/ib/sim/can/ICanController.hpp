// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

#include "CanDatatypes.hpp"
#include "ib/util/HandlerId.hpp"

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

    /*! Callback type to indicate that a CanFrameEvent has been received.
    *  Cf., \ref AddFrameHandler(FrameHandler);
    */
    using FrameHandler = CallbackT<CanFrameEvent>;

    /*! Callback type to indicate that the ::CanControllerState has changed.
    *  Cf., \ref AddStateChangeHandler(StateChangeHandler);
    */
    using StateChangeHandler = CallbackT<CanStateChangeEvent>;

    /*! Callback type to indicate that the controller ::CanErrorState has changed.
    *  Cf., \ref AddErrorStateChangeHandler(ErrorStateChangeHandler);
    */
    using ErrorStateChangeHandler = CallbackT<CanErrorStateChangeEvent>;

    /*! Callback type to indicate that a CanFrameTransmitEvent has been received.
    *  Cf., \ref AddFrameTransmitHandler(FrameTransmitHandler);
    */
    using FrameTransmitHandler = CallbackT<CanFrameTransmitEvent>;

public:
    virtual ~ICanController() = default;

    /*! \brief Configure the baud rate of the controller
     *
     * \param rate Baud rate for regular (non FD) CAN messages given
     * in bps; valid range: 0 to 2'000'000
     *
     * \param fdRate Baud rate for CAN FD messages given in bps; valid
     * range: 0 to 16'000'000
     *
     * NB: The baud rate of a CAN controller must be set before using it.
     * 
     * NB: In a detailed simulation, the baud rate is used to calculate
     * transmission delays of CAN messages and to determine proper
     * configuration and interoperation of the connected controllers.
     * 
     */
    virtual void SetBaudRate(uint32_t rate, uint32_t fdRate) = 0;

    /*! \brief Reset the CAN controller
     *
     * Resets the controller's Transmit Error Count (TEC) and the
     * Receive Error Count (REC). Furthermore, sets the
     * CAN controller state to CanControllerState::Uninit and the
     * controller's error state to CanErrorState::NotAvailable.
     *
     * \ref Start(), \ref Stop(), \ref Sleep()
     */
    virtual void Reset() = 0;

    /*! \brief Start the CAN controller
     *
     * \ref Reset(), \ref Stop(), \ref Sleep()
     */
    virtual void Start() = 0;

    /*! \brief Stop the CAN controller
     *
     * \ref Reset(), \ref Start(), \ref Sleep()
     */
    virtual void Stop() = 0;

    /*! \brief Put the CAN controller in sleep mode
     *
     * \ref Reset(), Start(), \ref Stop()
     */
    virtual void Sleep() = 0;

    /*! \brief Request the transmission of a CanFrame
     *
     * NB: The CanFrame must provide a valid CAN
     * ID and valid flags. The controller
     * must be in the Started state to transmit and receive messages.
     * 
     * \param userContext An optional user provided pointer that is
     * reobtained when receiving the message.
     *
     * \return Unique TX identifier to relate the request to following
     * CanFrameTransmitEvent messages.
     */
    virtual auto SendFrame(const CanFrame& msg, void* userContext = nullptr) -> CanTxId = 0;

    /*! \brief Register a callback for CAN message reception
     *
     * The registered handler is called when the controller receives a
     * new CanFrame.
     * 
     * \return Returns a \ref HandlerId that can be used to remove the callback.
     */
    virtual HandlerId AddFrameHandler(FrameHandler handler,
                                      DirectionMask directionMask = (DirectionMask)TransmitDirection::RX) = 0;

    /*! \brief Remove a FrameHandler by HandlerId on this controller 
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveFrameHandler(HandlerId handlerId) = 0;

    /*! \brief Register a callback for controller state changes
     *
     * The registered handler is called when the ::CanControllerState of
     * the controller changes. E.g., after starting the controller, the
     * state changes from CanControllerState::Uninit to
     * CanControllerState::Started.
     * 
     * \return Returns a \ref HandlerId that can be used to remove the callback.
     */
    virtual HandlerId AddStateChangeHandler(StateChangeHandler handler) = 0;

    /*! \brief Remove a StateChangeHandler by HandlerId on this controller 
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveStateChangeHandler(HandlerId handlerId) = 0;

    /*! \brief Register a callback for changes of the controller's error state
     *
     * The registered handler is called when the ::CanErrorState of the
     * controller changes. During normal operation, the controller
     * should be in state CanErrorState::ErrorActive. The states correspond
     * to the error state handling protocol of the CAN specification.
     *
     * NB: Only supported in a detailed simulation. In simple simulation,
     * the handler is never called.
     * 
     * \return Returns a \ref HandlerId that can be used to remove the callback.
     */
    virtual HandlerId AddErrorStateChangeHandler(ErrorStateChangeHandler handler) = 0;

    /*! \brief Remove an ErrorStateChangeHandler by HandlerId on this controller 
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveErrorStateChangeHandler(HandlerId handlerId) = 0;

    /*! \brief Register a callback for the TX status of sent CAN messages
     *
     * The registered handler is called when a CAN message was
     * successfully transmitted on the bus or when an error occurred.
     *
     * NB: Full support in a detailed simulation. In a simple simulation, all
     * messages are automatically positively acknowledged.
     */
    virtual HandlerId AddFrameTransmitHandler(FrameTransmitHandler handler, 
        CanTransmitStatusMask statusMask = (CanTransmitStatusMask)CanTransmitStatus::Transmitted 
            | (CanTransmitStatusMask)CanTransmitStatus::Canceled
            | (CanTransmitStatusMask)CanTransmitStatus::DuplicatedTransmitId
            | (CanTransmitStatusMask)CanTransmitStatus::TransmitQueueFull) = 0;

    /*! \brief Remove a FrameTransmitHandler by HandlerId on this controller 
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveFrameTransmitHandler(HandlerId handlerId) = 0;

};

} // namespace can
} // namespace sim
} // namespace ib

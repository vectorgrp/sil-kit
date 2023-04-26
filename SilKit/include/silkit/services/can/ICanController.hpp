/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <functional>

#include "CanDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Can {

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
    *  Cf., \ref ICanController::AddFrameHandler(FrameHandler,DirectionMask);
    */
    using FrameHandler = CallbackT<CanFrameEvent>;

    /*! Callback type to indicate that the ::CanControllerState has changed.
    *  Cf., \ref ICanController::AddStateChangeHandler(StateChangeHandler);
    */
    using StateChangeHandler = CallbackT<CanStateChangeEvent>;

    /*! Callback type to indicate that the controller ::CanErrorState has changed.
    *  Cf., \ref ICanController::AddErrorStateChangeHandler(ErrorStateChangeHandler);
    */
    using ErrorStateChangeHandler = CallbackT<CanErrorStateChangeEvent>;

    /*! Callback type to indicate that a CanFrameTransmitEvent has been received.
    *  Cf., \ref ICanController::AddFrameTransmitHandler(FrameTransmitHandler,CanTransmitStatusMask);
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
     * \param xlRate Baud rate for CAN XL messages given in bps; valid
     * range: 0 to 16'000'000
     *
     * NB: The baud rate of a CAN controller must be set before using it.
     * 
     * NB: In a detailed simulation, the baud rate is used to calculate
     * transmission delays of CAN messages and to determine proper
     * configuration and interoperation of the connected controllers.
     * 
     */
    virtual void SetBaudRate(uint32_t rate, uint32_t fdRate, uint32_t xlRate) = 0;

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
     * \param msg The frame to transmit.
     * \param userContext An optional user provided pointer that is
     * reobtained in the \ref FrameTransmitHandler.
     */
    virtual void SendFrame(const CanFrame& msg, void* userContext = nullptr) = 0;

    /*! \brief Register a callback for CAN message reception
     *
     * The registered handler is called when the controller receives a
     * new CanFrame.
     * 
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddFrameHandler(FrameHandler handler,
                                      DirectionMask directionMask = (DirectionMask)TransmitDirection::RX) -> HandlerId = 0;

    /*! \brief Remove a FrameHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveFrameHandler(HandlerId handlerId) = 0;

    /*! \brief Register a callback for controller state changes
     *
     * The registered handler is called when the \ref SilKit::Services::Can::CanControllerState of
     * the controller changes. E.g., after starting the controller, the
     * state changes from \ref SilKit::Services::Can::CanControllerState::Uninit to
     * \ref SilKit::Services::Can::CanControllerState::Started.
     * 
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddStateChangeHandler(StateChangeHandler handler) -> HandlerId = 0;

    /*! \brief Remove a StateChangeHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveStateChangeHandler(HandlerId handlerId) = 0;

    /*! \brief Register a callback for changes of the controller's error state
     *
     * The registered handler is called when the \ref SilKit::Services::Can::CanErrorState of the
     * controller changes. During normal operation, the controller
     * should be in state \ref SilKit::Services::Can::CanErrorState::ErrorActive. The states correspond
     * to the error state handling protocol of the CAN specification.
     *
     * NB: Only supported in a detailed simulation. In simple simulation,
     * the handler is never called.
     * 
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddErrorStateChangeHandler(ErrorStateChangeHandler handler) -> HandlerId = 0;

    /*! \brief Remove an ErrorStateChangeHandler by \ref SilKit::Util::HandlerId on this controller
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
    virtual auto AddFrameTransmitHandler(FrameTransmitHandler handler,
                                         CanTransmitStatusMask statusMask = SilKit_CanTransmitStatus_DefaultMask)
        -> HandlerId = 0;

    /*! \brief Remove a FrameTransmitHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveFrameTransmitHandler(HandlerId handlerId) = 0;
};

} // namespace Can
} // namespace Services
} // namespace SilKit

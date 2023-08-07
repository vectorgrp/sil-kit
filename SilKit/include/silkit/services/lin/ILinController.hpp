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
#include <string>

#include "LinDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

/*! 
 * The LIN controller can assume the role of a LIN master or a LIN slave. All LIN nodes must be configured with their
 * respective role using \ref ILinController::Init(LinControllerConfig) to perform data transmission and reception.
 *
 * AUTOSAR-like LIN interface:
 *
 * - \ref SendFrame() initiates a frame transfer for a given \ref LinFrameResponseType.
 * For LinFrameResponseType::MasterResponse, the controller doesn't need to be configured for transmission on this LIN ID.
 * Requires \ref LinControllerMode::Master.
 *
 * non-AUTOSAR LIN interface:
 *
 * - \ref SendFrameHeader() initiates the transmission of a LIN frame for a particular LIN identifier. For a successful
 * transmission, exactly one LIN slave or master must have previously set a corresponding frame response for 
 * unconditional transmission. Requires \ref LinControllerMode::Master.
 * - \ref UpdateTxBuffer() updates the response data for a particular LIN identifier. Can be used with 
 * \ref LinControllerMode::Master and \ref LinControllerMode::Slave.
 * 
 */
class ILinController
{
public:

    /*! \brief Generic LIN callback method
    */
    template <typename MsgT>
    using CallbackT = std::function<void(ILinController* controller, const MsgT& msg)>;

    /*! Callback type to indicate the end of a LIN LinFrame transmission.
     *  Cf., \ref AddFrameStatusHandler(FrameStatusHandler);
     */
    using FrameStatusHandler = CallbackT<LinFrameStatusEvent>;

    /*! Callback type to indicate that a go-to-sleep frame has been received.
     *  Cf., \ref AddGoToSleepHandler(GoToSleepHandler);
     */
    using GoToSleepHandler = CallbackT<LinGoToSleepEvent>;

    /*! Callback type to indicate that a wakeup pulse has been received.
     *  Cf., \ref AddWakeupHandler(WakeupHandler);
     */
    using WakeupHandler = CallbackT<LinWakeupEvent>;

public:
    virtual ~ILinController() = default;

    /*! \brief Initialize the LIN controller defining its role and RX/TX configuration. 
     * 
     * All controllers must be initialized exactly once to take part in LIN communication.
     * 
     * \param config The controller configuration contains:
     *  - controllerMode, either sets LIN master or LIN slave mode.
     *  - baudRate, determine transmission speeds (only used for detailed simulation).
     *  - frameResponses, a vector of LinFrameResponse. This must contain the final configuration
     * on which LIN Ids the controller will receive (LinFrameResponseMode::Rx) or respond to
     * (LinFrameResponseMode::TxUnconditional) frames. An exception is the use of \ref SendFrame() for 
     * LinFrameResponseType::MasterResponse, which allows to extend the configuration during operation. 
     *
     * *AUTOSAR Name:* Lin_Init
     * 
     * \throws SilKit::StateError if the LIN Controller is configured with LinControllerMode::Inactive
     * \throws SilKit::StateError if Init() is called a second time on this LIN Controller.
     * \throws SilKit::StateError if InitDynamic() has already been called on the LIN Controller.
     */
    virtual void Init(LinControllerConfig config) = 0;

    //! \brief Get the current status of the LIN Controller, i.e., Operational or Sleep.
    virtual auto Status() const noexcept -> LinControllerStatus = 0;

    /*! \brief Initiate a LIN data transfer of a given LinFrameResponseType (AUTOSAR LIN master interface)
     *
     * The responseType determines if frame.data is used for the frame response or if a different node has to provide 
     * it:
     * - MasterResponse: \ref SilKit::Services::Lin::LinFrame is sent from this controller to all connected slaves using
     *   frame.data. The LIN Master doesn't have to be configured with LinFrameResponseMode::TxUnconditional on this LIN
     *   ID.
     * - SlaveResponse: the frame response must be provided by a connected slave and is received by this controller.
     * - SlaveToSlave: the frame response must be provided by a connected slave and is ignored by this controller.
     *
     * *AUTOSAR Name:* Lin_SendFrame
     *
     * \param frame Provides the LIN identifier, checksum model, and optional data.
     * \param responseType Determines which LIN Node will provide the frame response.
     * 
     * \throws SilKit::StateError if the LIN Controller is not initialized or not a master node.
     * \throws SilKit::StateError if InitDynamic() has been called on the LIN Controller.
     */
    virtual void SendFrame(LinFrame frame, LinFrameResponseType responseType) = 0;

    /*! \brief Initiate a LIN data transfer by sending a LIN header (AUTOSAR LIN master interface)
     * 
     * \param linId Provides the LIN header identifier. The node that is configured to respond on this ID will complete
     * the transmission and provide the response data.
     *
     * \throws SilKit::StateError if the LIN Controller is not initialized or not a master node.
     */
    virtual void SendFrameHeader(LinId linId) = 0;

    /*! \brief Update the response data. The LIN controller needs to be configured with TxUnconditional on this ID.
     * 
     * \param frame Provides the LIN ID and data used for the update.
     * 
     * \throws SilKit::StateError if the LIN Controller is not initialized.
     * \throws SilKit::ConfigurationError if the LIN Controller is not configured with TxUnconditional on this ID.
     * \throws SilKit::StateError if InitDynamic() has been called on the LIN Controller.
     */
    virtual void UpdateTxBuffer(LinFrame frame) = 0;

    /*! \brief Set a RX/TX configuration during operation.
     *
     * \param response The frame and response mode to be configured.
     *
     * \throws SilKit::StateError if the LIN Controller is not initialized.
     * \throws SilKit::StateError if InitDynamic() has been called on the LIN Controller.
     */
    virtual void SetFrameResponse(LinFrameResponse response) = 0;

    /*! \brief Transmit a go-to-sleep-command and set ControllerState::Sleep and enable wake-up
     *
     * *AUTOSAR Name:* Lin_GoToSleep
     * 
     * \throws SilKit::StateError if the LIN Controller is not initialized or not a master node.
     */
    virtual void GoToSleep() = 0;

    /*! \brief Set ControllerState::Sleep without sending a go-to-sleep command.
     *
     * *AUTOSAR Name:* Lin_GoToSleepInternal
     * 
     * \throws SilKit::StateError if the LIN Controller is not initialized.
     */
    virtual void GoToSleepInternal() = 0;

    /*! \brief Generate a wake up pulse and set ControllerState::Operational.
     *
     * *AUTOSAR Name:* Lin_Wakeup
     * 
     * \throws SilKit::StateError if the LIN Controller is not initialized.
     */
    virtual void Wakeup() = 0;

    /*! Set ControllerState::Operational without generating a wake up pulse.
     *
     * *AUTOSAR Name:* Lin_WakeupInternal
     * 
     * \throws SilKit::StateError if the LIN Controller is not initialized.
     */
    virtual void WakeupInternal() = 0;

    /*! \brief Reports the \ref SilKit::Services::Lin::LinFrameStatus of a LIN \ref SilKit::Services::Lin::LinFrame
     *         transmission and provides the transmitted frame.
     *
     * The FrameStatusHandler is used for reception and acknowledgement of LIN frames. The direction (prefixed with 
     * LIN_TX_ or LIN_RX_) and error state of the tranmission is encoded in the
     * \ref SilKit::Services::Lin::LinFrameStatus.
     * 
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddFrameStatusHandler(FrameStatusHandler handler) -> HandlerId = 0;

    /*! \brief Remove a FrameStatusHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveFrameStatusHandler(HandlerId handlerId) = 0;

    /*! \brief The GoToSleepHandler is called whenever a go-to-sleep frame
     * is received.
     *
     * Note: The LIN controller does not automatically enter sleep
     * mode on reception of a go-to-sleep frame. I.e.,
     * GoToSleepInternal() must be called manually.
     *
     * Note: This handler will always be called, independently of the
     * \ref SilKit::Services::Lin::LinFrameResponseMode configuration for LIN ID 0x3C. However,
     * regarding the FrameStatusHandler, the go-to-sleep frame is
     * treated like every other frame, i.e. the FrameStatusHandler is
     * only called for LIN ID 0x3C if configured as
     * LinFrameResponseMode::Rx.
     * 
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddGoToSleepHandler(GoToSleepHandler handler) -> HandlerId = 0;

    /*! \brief Remove a GoToSleepHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveGoToSleepHandler(HandlerId handlerId) = 0;

    /*! \brief The WakeupHandler is called whenever a wake up pulse is received
     *
     * Note: The LIN controller does not automatically enter
     * operational mode on wake up pulse detection. I.e.,
     * WakeInternal() must be called manually.
     * 
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddWakeupHandler(WakeupHandler handler) -> HandlerId = 0;

    /*! \brief Remove a WakeupHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveWakeupHandler(HandlerId handlerId) = 0;
};

} // namespace Lin
} // namespace Services
} // namespace SilKit



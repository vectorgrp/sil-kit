// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <string>

#include "LinDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

/*! 
 * The LIN controller can assume the role of a LIN master or a LIN
 * slave. It provides two kinds of interfaces to perform data
 * transfers and provide frame responses:
 *
 * AUTOSAR-like LIN master interface:
 *
 * - \ref SendFrame() transfers a frame from or to a LIN
 * master. Requires \ref LinControllerMode::Master.
 *
 *
 * non-AUTOSAR interface:
 *
 * - \ref SetFrameResponse() configures
 * the response for a particular LIN identifier. Can be used with \ref
 * LinControllerMode::Master and \ref LinControllerMode::Slave.
 *
 * - \ref SendFrameHeader() initiates the transmission of a
 * LIN frame for a particular LIN identifier. For a successful
 * transmission, exactly one LIN slave or master must have previously
 * set a corresponding frame response for unconditional
 * transmission. Requires \ref LinControllerMode::Master.
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

    /*! Callback type to indicate that a frame response update has been received.
     *  Cf., \ref AddFrameResponseUpdateHandler(WakeupHandler);
     */
    using FrameResponseUpdateHandler = CallbackT<LinFrameResponseUpdateEvent>;

public:
    virtual ~ILinController() = default;

    /*! \brief Initialize the LIN controller
     * 
     * \param config The Controller configuration contains:
     *  - controllerMode, either sets LIN master or LIN slave mode
     *  - baudRate, determine transmission speeds (only used for VIBE simulation)
     *  - frameResponses, an optional set of initial FrameResponses
     *
     * *AUTOSAR Name:* Lin_Init
     */
    virtual void Init(LinControllerConfig config) = 0;

    //! \brief Get the current status of the LIN Controller, i.e., Operational or Sleep.
    virtual auto Status() const noexcept -> LinControllerStatus = 0;

    /*! \brief AUTOSAR LIN master interface
     *
     * Perform a full LIN data transfer, i.e., frame header + frame
     * response. The responseType determines if frame.data is used for
     * the frame response or if a different node has to provide it:
     *
     * \li MasterResponse: \ref LinFrame is sent from this controller to
     *     all connected slaves.
     * \li SlaveResponse: the frame response must be provided by a
     *     connected slave and is received by this controller.
     * \li SlaveToSlave: the frame response must be provided by a
     *     connected slave and is ignored by this controller.
     *
     * *AUTOSAR Name:* Lin_SendFrame
     *
     * \param frame provides the LIN identifier, checksum model, and optional data
     * \param responseType determines if *frame.data* must is used for the frame response.
     */
    virtual void SendFrame(LinFrame frame, LinFrameResponseType responseType) = 0;

    //! Send Interface for a non-AUTOSAR LIN Master
    virtual void SendFrameHeader(LinIdT linId) = 0;

    /*! LinFrameResponse configuration for Slaves or non-AUTOSAR LIN
     *  Masters The corresponding LIN ID does not need to be
     *  previously configured. */
    virtual void SetFrameResponse(LinFrame frame, LinFrameResponseMode mode) = 0;

    /*! LinFrameResponse configuration for Slaves or non-AUTOSAR LIN Masters.
     * 
     * Configures multiple responses at once. Corresponding IDs do not
     * need to be previously configured.
     *
     * NB: only configures responses for the provided LIN IDs. I.e.,
     * an empty vector does not clear or reset the currently
     * configured FrameResponses.
     */
    virtual void SetFrameResponses(std::vector<LinFrameResponse> responses) = 0;

    /*! \brief Transmit a go-to-sleep-command and set ControllerState::Sleep and enable wake-up
     *
     * *AUTOSAR Name:* Lin_GoToSleep
     * \throw ib::StateError Command issued with wrong LinControllerMode
     */
    virtual void GoToSleep() = 0;
    /*! \brief Set ControllerState::Sleep without sending a go-to-sleep command.
     *
     * *AUTOSAR Name:* Lin_GoToSleepInternal
     * \throw ib::StateError Command issued with wrong LinControllerMode
     */
    virtual void GoToSleepInternal() = 0;
    /*! \brief Generate a wake up pulse and set ControllerState::Operational.
     *
     * *AUTOSAR Name:* Lin_Wakeup
     * \throw ib::StateError Command issued with wrong LinControllerMode
     */
    virtual void Wakeup() = 0;
    /*! Set ControllerState::Operational without generating a wake up pulse.
     *
     * *AUTOSAR Name:* Lin_WakeupInternal
     * \throw ib::StateError Command issued with wrong LinControllerMode
     */
    virtual void WakeupInternal() = 0;

    /*! \brief Report the \ref LinFrameStatus of a LIN \ref LinFrame
     * transmission and provides the transmitted frame.
     *
     * The FrameStatusHandler is called once per call to
     * SendFrame() or call to
     * SendFrameHeader(). The handler is called independently
     * of the transmission's success or failure.
     *
     * The FrameStatusHandler is called for all participating LIN
     * controllers. I.e., for LIN masters, it is always called, and
     * for LIN slaves, it is called if the corresponding \ref LinIdT is
     * configured SlaveFrameResponseMode::Rx or
     * SlaveFrameResponseMode::TxUnconditional.
     *
     * <em>Note: this is one of the major changes to the previous version.
     * Previously, frame transmission was indicated using different
     * means. For Masters, a TX was confirmed using the
     * TxCompleteHandler while an RX was handled using
     * ReceiveMessageHandler. For LIN slaves the confirmation varied
     * for simple simulation and VIBE simulation.</em>
     * 
     * \return Returns a \ref HandlerId that can be used to remove the callback.
     */
    virtual HandlerId AddFrameStatusHandler(FrameStatusHandler handler) = 0;

    /*! \brief Remove a FrameStatusHandler by HandlerId on this controller 
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveFrameStatusHandler(HandlerId handlerId) = 0;

    /*! \brief The GoToSleepHandler is called whenever a go-to-sleep frame
     * was received.
     *
     * Note: The LIN controller does not automatically enter sleep
     * mode up reception of a go-to-sleep frame. I.e.,
     * GoToSleepInternal() must be called manually
     *
     * NB: This handler will always be called, independently of the
     * \ref LinFrameResponseMode configuration for LIN ID 0x3C. However,
     * regarding the FrameStatusHandler, the go-to-sleep frame is
     * treated like every other frame, i.e. the FrameStatusHandler is
     * only called for LIN ID 0x3C if configured as
     * LinFrameResponseMode::Rx.
     * 
     * \return Returns a \ref HandlerId that can be used to remove the callback.
     */
    virtual HandlerId AddGoToSleepHandler(GoToSleepHandler handler) = 0;

    /*! \brief Remove a GoToSleepHandler by HandlerId on this controller 
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
     * \return Returns a \ref HandlerId that can be used to remove the callback.
     */
    virtual HandlerId AddWakeupHandler(WakeupHandler handler) = 0;

    /*! \brief Remove a WakeupHandler by HandlerId on this controller 
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveWakeupHandler(HandlerId handlerId) = 0;

    /*! \brief The FrameResponseUpdateHandler provides direct access
     * to the LinFrameResponse configuration of other LIN controllers.
     *
     * NB: This callback is mainly for diagnostic purposes and is NOT
     * needed for regular LIN controller operation.
     * 
     * \return Returns a \ref HandlerId that can be used to remove the callback.
     */
    virtual HandlerId AddFrameResponseUpdateHandler(FrameResponseUpdateHandler handler) = 0;

    /*! \brief Remove a FrameResponseUpdateHandler by HandlerId on this controller 
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveFrameResponseUpdateHandler(HandlerId handlerId) = 0;
};

} // namespace lin
} // namespace sim
} // namespace ib



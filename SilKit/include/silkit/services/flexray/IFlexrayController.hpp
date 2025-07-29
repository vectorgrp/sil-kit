// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <functional>

#include "FlexrayDatatypes.hpp"
#include "silkit/util/HandlerId.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

/*! \brief Abstract FlexRay Controller API to be used by vECUs
 */
class IFlexrayController
{
public:
    /*! \brief Generic FleyRay callback method
    */
    template <typename MsgT>
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
     */
    using CycleStartHandler = CallbackT<FlexrayCycleStartEvent>;

public:
    virtual ~IFlexrayController() = default;

    //! \brief Configure the controller and switch to Ready state
    virtual void Configure(const FlexrayControllerConfig& config) = 0;

    //! \brief Reconfigure a TX Buffer that was previously setup with IFlexrayController::Configure(const FlexrayControllerConfig&)
    virtual void ReconfigureTxBuffer(uint16_t txBufferIdx, const FlexrayTxBufferConfig& config) = 0;

    /*! \brief Update the content of a previously configured TX buffer.
     *
     * A FlexRay message will be sent at the time matching to
     * the configured Slot ID. If the buffer was configured with FlexrayTransmissionMode::SingleShot,
     * the content is sent exactly once. If it is configured as FlexrayTransmissionMode::Continuous,
     * the content is sent repeatedly according to the offset and repetition configuration.
     *
     *
     *  \see IFlexrayController::Configure(const FlexrayControllerConfig&)
     */
    virtual void UpdateTxBuffer(const FlexrayTxBufferUpdate& update) = 0;

    /*! \brief Send the FlexrayChiCommand::RUN
    */
    virtual void Run() = 0;

    /*! \brief Send the FlexrayChiCommand::DEFERRED_HALT
    */
    virtual void DeferredHalt() = 0;

    /*! \brief Send the FlexrayChiCommand::FREEZE
    */
    virtual void Freeze() = 0;

    /*! \brief Send the FlexrayChiCommand::ALLOW_COLDSTART
    */
    virtual void AllowColdstart() = 0;

    /*! \brief Send the FlexrayChiCommand::ALL_SLOTS
    */
    virtual void AllSlots() = 0;

    //! \brief Send the FlexrayChiCommand::WAKEUP
    virtual void Wakeup() = 0;

    /*! \brief Receive a FlexRay message from a different controller.
     * 
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddFrameHandler(FrameHandler handler) -> HandlerId = 0;

    /*! \brief Remove a FrameHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveFrameHandler(HandlerId handlerId) = 0;

    /*! \brief Notification that a FlexRay message has been successfully sent.
     * 
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddFrameTransmitHandler(FrameTransmitHandler handler) -> HandlerId = 0;

    /*! \brief Remove a FrameTransmitHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveFrameTransmitHandler(HandlerId handlerId) = 0;

    /*! \brief Notification that a wakeup has been received.
     * 
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddWakeupHandler(WakeupHandler handler) -> HandlerId = 0;

    /*! \brief Remove a WakeupHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveWakeupHandler(HandlerId handlerId) = 0;

    /*! \brief Notification that the POC status has changed.
     * 
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddPocStatusHandler(PocStatusHandler handler) -> HandlerId = 0;

    /*! \brief Remove a PocStatusHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemovePocStatusHandler(HandlerId handlerId) = 0;

    /*! \brief Notification that the controller has received a symbol.
     *
     * This callback is primarily intended for tracing. There is no need to react on it.
     * The symbols relevant for interaction trigger also an additional callback,
     * e.g., \ref WakeupHandler.
     * 
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddSymbolHandler(SymbolHandler handler) -> HandlerId = 0;

    /*! \brief Remove a SymbolHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveSymbolHandler(HandlerId handlerId) = 0;

    /*! \brief Notification that the controller has sent a symbol.
     *
     * This callback is primarily intended for tracing. There is no need to react on it.
     * Currently, the following SymbolPatterns can occur:
     *  - Wakeup() will cause sending the FlexraySymbolPattern::Wus, if the bus is idle.
     *  - Run() will cause the transmission of FlexraySymbolPattern::CasMts if configured to coldstart the bus.
     * 
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddSymbolTransmitHandler(SymbolTransmitHandler handler) -> HandlerId = 0;

    /*! \brief Remove a SymbolTransmitHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveSymbolTransmitHandler(HandlerId handlerId) = 0;

    /*! \brief Notification that a new FlexRay cycle did start.
     *
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddCycleStartHandler(CycleStartHandler handler) -> HandlerId = 0;

    /*! \brief Remove a CycleStartHandler by \ref SilKit::Util::HandlerId on this controller
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveCycleStartHandler(HandlerId handlerId) = 0;
};

} // namespace Flexray
} // namespace Services
} // namespace SilKit

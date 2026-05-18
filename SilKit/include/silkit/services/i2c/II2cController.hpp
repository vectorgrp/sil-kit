// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>

#include "I2cDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

/*! \brief Abstract I2C Controller API to be used by vECUs */
class II2cController
{
public:
    /*! \brief Generic I2C callback type */
    template <typename MsgT>
    using CallbackT = std::function<void(II2cController* controller, const MsgT& msg)>;

    /*! Callback type invoked when an I2C frame is observed on the bus. */
    using FrameHandler = CallbackT<I2cFrameEvent>;

    /*! Callback type invoked when a frame transmission completes or fails. */
    using FrameTransmitHandler = CallbackT<I2cFrameTransmitEvent>;

public:
    virtual ~II2cController() = default;

    /*! \brief Initialize the controller with the given configuration.
     *
     * Must be called before SendFrame or SetReadResponse. Broadcasts the controller's
     * configuration to other participants on the same network so that slave address
     * routing can be established.
     *
     * \param config Controller configuration (mode, slave address, speed mode).
     */
    virtual void Init(I2cControllerConfig config) = 0;

    /*! \brief Initiate an I2C transfer (master only).
     *
     * For Write transfers, \c frame.data contains the payload.
     * For Read transfers, \c frame.data should be empty; the response data is delivered
     * via the frame handler.
     *
     * \param frame       The I2C frame to transmit.
     * \param userContext Optional user pointer reobtained in the frame transmit handler.
     */
    virtual void SendFrame(const I2cFrame& frame, void* userContext = nullptr) = 0;

    /*! \brief Pre-load the read-response buffer for a slave controller.
     *
     * When a master issues a Read to this slave's address, the provided data
     * is returned as the slave's response. Must be called before the master
     * issues the Read.
     *
     * \param data Buffer containing the data to return on the next Read request.
     */
    virtual void SetReadResponse(Util::Span<const uint8_t> data) = 0;

    /*! \brief Register a callback for I2C frame reception.
     *
     * \param handler        The handler called on each observed frame.
     * \param directionMask  Bitmask of \ref TransmitDirection values to filter TX, RX, or both.
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddFrameHandler(FrameHandler handler,
                                 DirectionMask directionMask = (DirectionMask)TransmitDirection::RX)
        -> HandlerId = 0;

    /*! \brief Remove a FrameHandler by \ref SilKit::Util::HandlerId on this controller. */
    virtual void RemoveFrameHandler(HandlerId handlerId) = 0;

    /*! \brief Register a callback for I2C frame transmit events (master only).
     *
     * \param handler The handler called when a transfer completes or fails.
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddFrameTransmitHandler(FrameTransmitHandler handler) -> HandlerId = 0;

    /*! \brief Remove a FrameTransmitHandler by \ref SilKit::Util::HandlerId on this controller. */
    virtual void RemoveFrameTransmitHandler(HandlerId handlerId) = 0;
};

} // namespace I2c
} // namespace Services
} // namespace SilKit

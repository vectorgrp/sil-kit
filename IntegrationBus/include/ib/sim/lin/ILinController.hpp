// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

#include "LinDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

/*! \brief Abstract LIN Controller API to be used by vECUs
 */
class ILinController
{
public:
    template<typename MsgT>
    using CallbackT = std::function<void(ILinController* controller, const MsgT& msg)>;
    using ReceiveMessageHandler = CallbackT<LinMessage>;
    using TxCompleteHandler     = CallbackT<MessageStatus>;

public:
    virtual ~ILinController() = default;

    /*! \brief Configure the controller as a LIN master
     *
     * Configures the controller as a LIN master and activates the
     * controller.
     */
    virtual void SetMasterMode() = 0;

    /*! \brief Configure the controller as a LIN slave
     *
     * Configures the controller as a LIN slave and activates the
     * controller.
     */
    virtual void SetSlaveMode() = 0;
    
    /*! \brief Configure the baud rate of the controller
     *
     * \param rate the baud rate to be set given in kHz
     */
    virtual void SetBaudRate(uint32_t rate) = 0;

    /*! \brief Configure the responses of a LIN slave
     *
     * The SlaveConfiguration contains a vector of
     * SlaveResponseConfigs. The length of the vector determines the
     * range of relevant LIN IDs for the slave. Each entry corresponds
     * to a LIN ID and states how the LIN ID shall be treated (\ref
     * ResponseMode), the expected checksum model (\ref
     * ChecksumModel), and the expected payload length in bytes.
     *
     * The SlaveConfiguration does not contain payload data for
     * responses. Response payload must be set using \ref
     * SetResponse(LinId, const Payload&) per LIN ID.
     */
    virtual void SetSlaveConfiguration(const SlaveConfiguration& config) = 0;

    /*! \brief Set the response payload for a given LIN ID
     *
     * Set \param payload as the response for requests with LIN ID
     * \param linId. The corresponding LIN ID must be previoiusly
     * configured as ResponseMode::TxUnconditional, cf. \ref
     * SetSlaveConfiguration(const SlaveConfiguration&).
     */
    virtual void SetResponse(LinId linId, const Payload& payload) = 0;

    /*! \brief Set the response payload for a given LIN ID and update checksum configuration
    *
    * Set \param payload as the response for requests with LIN ID
    * \param linId. Also, the configured checksum model is updated
    * to \param checksumModel.
    * NB: The corresponding LIN ID must be previoiusly
    * configured as ResponseMode::TxUnconditional, cf. \ref
    * SetSlaveConfiguration(const SlaveConfiguration&).
    */
    virtual void SetResponseWithChecksum(LinId linId, const Payload& payload, ChecksumModel checksumModel) = 0;

    /*! \brief Mark a LIN ID as unused
     *
     * NB: currently not supported in VIBE LIN simulation.
     */
    virtual void RemoveResponse(LinId linId) = 0;


    /*! \brief Send a LIN message to the connected slaves
     *
     * The following fields of the message must be set:
     *  - linId (always)
     *  - payload (always)
     *  - checksumModel (VIBE)
     *
     * NB: in VIBE simulation, SendMessage(const LinMessage&) must not
     * be called when there is still an ongoing transmission on the
     * LIN bus. Otherwise, the SendMessage call will be answered with
     * MessageStatus::Busy (cf. \ref
     * RegisterTxCompleteHandler(TxCompleteHandler)). This limitation
     * is not enforced in the simple simulation.
     *
     * NB: Must only be called when configured as a LIN master!
     */
    virtual void SendMessage(const LinMessage& msg) = 0;
    
    /*! \brief Request a LIN message from the connected slaves
     *
     * The following fields of the request must be set:
     *  - linId (always)
     *  - payloadLength (VIBE)
     *  - checksummodel (VIBE)
     *
     * NB: in VIBE simulation, RequesetMessage(const RxRequest&) must not
     * be called when there is still an ongoing transmission on the
     * LIN bus. Otherwise, the SendMessage call will be answered with
     * MessageStatus::Busy (cf. \ref
     * RegisterTxCompleteHandler(TxCompleteHandler)). This limitation
     * is not enforced in the simple simulation.
     *
     * NB: Must only be called when configured as a LIN master!
     */
    virtual void RequestMessage(const RxRequest& request) = 0;

    /*! \brief Register a callback for TX completion
     *
     * The handler is called when the previously sent LinMessage was
     * successfully transmitted on the LIN bus. Check MessageStatus to
     * see if the bus is free and a new message can be sent or
     * requested.
     * 
     * NB: supported in simple LIN simulation and in VIBE LIN
     * simulation.
     *
     * NB: Must only be called when configured as a LIN master!
     */
    virtual void RegisterTxCompleteHandler(TxCompleteHandler handler) = 0;

    /*! \brief Register a callback for LIN message reception
     *
     * The handler is called when the master received a reply from a
     * LIN slave following a previous call to \ref
     * RequestMessage(const RxRequest&).
     *
     * NB: Must only be called when configured as a LIN master!
     */
    virtual void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) = 0;
};
    
} // namespace lin
} // namespace sim
} // namespace ib

// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbReceiver.hpp"
#include "ib/mw/IIbSender.hpp"

#include "FrDatatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief IIbToFrBusSimulator interface
 *
 *  Used by the ComAdapter
 */
class IIbToFrBusSimulator
    : public mw::IIbReceiver<HostCommand, ControllerConfig, TxBufferUpdate>
    , public mw::IIbSender<FrMessage, FrMessageAck, FrSymbol, FrSymbolAck, ControllerStatus>
{
public:
    ~IIbToFrBusSimulator() = default;

    /* NB: There is no setter or getter for an EndpointAddress of the bus
     * simulator, since the Network Simulator manages multiple controllers with
     * different endpoints. I.e., the Network Simulator is aware of the endpointIds.
     */
    //! \brief Setter and getter for the ParticipantID associated with this CAN Network Simulator
    virtual void SetParticipantId(ib::mw::ParticipantId participantId) = 0;
    virtual auto GetParticipantId() const -> ib::mw::ParticipantId = 0;
};

} // namespace fr
} // namespace sim
} // namespace ib

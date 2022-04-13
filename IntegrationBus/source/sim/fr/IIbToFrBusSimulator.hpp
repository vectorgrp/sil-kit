// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "IIbServiceEndpoint.hpp"

#include "ib/sim/fr/FrDatatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief IIbToFrBusSimulator interface
 *
 *  Used by the Participant
 */
class IIbToFrBusSimulator
    : public mw::IIbReceiver<HostCommand, ControllerConfig, TxBufferConfigUpdate, TxBufferUpdate>
    , public mw::IIbSender<FrMessage, FrMessageAck, FrSymbol, FrSymbolAck, CycleStart, PocStatus>
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
